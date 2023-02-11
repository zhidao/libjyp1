/* libjyp1: Linux用SANWA SUPPLY USBゲームパッドJY-P1操作ライブラリ
 *
 * 2005. 9. 4. Created. : Zhidao
 * 2023. 2.11. Last updated. : Zhidao
 */

#include <linux/version.h>
#include <jyp1.h>
#include <errno.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
/* for 2.6 and later */
#define JYP1_INIT_NUM 8
#else
/* for 2.4 */
#define JYP1_INIT_NUM 29
#endif

/* *** イベント管理 *** */

/* 十字ボタンイベント */
static char _jyp1_axis_event(unsigned char id, short value)
{
  switch( id ){
  case 0x0: /* horizontal */
    return value > 0 ? JYP1_EVENT_RIGHT :
      ( value < 0 ? JYP1_EVENT_LEFT : JYP1_EVENT_HRZ_NEUTRAL );
  case 0x1: /* vertical */
    return value > 0 ? JYP1_EVENT_DOWN :
      ( value < 0 ? JYP1_EVENT_UP : JYP1_EVENT_VRT_NEUTRAL );
  default: ;
  }
  fprintf( stderr, "invalid value of cross button event.\n" );
  return JYP1_EVENT_NONE;
}

/* その他ボタンイベント */
static char _jyp1_button_event(unsigned char id, short value)
{
  switch( id ){
  case 0x0: /* A */
    return value ? JYP1_EVENT_A_PRESSED : JYP1_EVENT_A_RELEASED;
  case 0x1: /* B */
    return value ? JYP1_EVENT_B_PRESSED : JYP1_EVENT_B_RELEASED;
  case 0x2: /* C */
    return value ? JYP1_EVENT_C_PRESSED : JYP1_EVENT_C_RELEASED;
  case 0x3: /* D */
    return value ? JYP1_EVENT_D_PRESSED : JYP1_EVENT_D_RELEASED;
  case 0x4: /* START */
    return value ? JYP1_EVENT_START_PRESSED : JYP1_EVENT_START_RELEASED;
  case 0x5: /* SELECT */
    return value ? JYP1_EVENT_SELECT_PRESSED : JYP1_EVENT_SELECT_RELEASED;
  case 0xb: /* TURBO */
    return value ? JYP1_EVENT_TURBO_RELEASED: JYP1_EVENT_TURBO_PRESSED;
  default: ;
  }
  fprintf( stderr, "invalid value of button event.\n" );
  return JYP1_EVENT_NONE;
}

/* 単独イベント受け付け。ヘッダに定義されている、各イベントに対応するcharを返す */
static char _jyp1_event(jyp1_t *jyp1)
{
  static struct js_event e;

  jyp1_read( jyp1, &e );
  if( e.type == JS_EVENT_BUTTON )
    return _jyp1_button_event( e.number, e.value );
  if( e.type == JS_EVENT_AXIS )
    return _jyp1_axis_event( e.number, e.value );
  return JYP1_EVENT_NONE;
}

/* *** コマンドリスト *** */

/* コマンドリストの初期化 */
static void _jyp1_com_init(jyp1_t *jyp1)
{
  memset( &jyp1->com, 0, sizeof(jyp1_com_t) );
  jyp1->com.next = NULL;
}

/* コマンドリストの破棄 */
static void _jyp1_com_destroy(jyp1_t *jyp1)
{
  jyp1_com_t *cp, *np;

  for( cp=jyp1->com.next; cp; cp=np ){
    np = cp->next;
    free( cp );
  }
  jyp1->com.next = NULL;
}

/* 全コマンドのprogressクリア */
static void _jyp1_com_clear(jyp1_t *jyp1)
{
  jyp1_com_t *com;

  for( com=jyp1->com.next; com; com=com->next )
    com->progress = 0;
}

/* コマンドの登録 (後から登録されたコマンドほど優先度が高い) */
/* cmd: コマンドを表すchar列
 * exclusive: 排他的コマンドか否か
 * action: 対応するアクション(コールバック関数)
 */
int jyp1_com_entry(jyp1_t *jyp1, char *cmd, bool exclusive, int (*action)(void*))
{
  jyp1_com_t *com;

  for( com=&jyp1->com; com->next; com=com->next );
  if( !( com->next = malloc( sizeof(jyp1_com_t) ) ) ){
    fprintf( stderr, "%s: cannot allocate memory.\n", __FUNCTION__ );
    return -1;
  }
  com->next->progress = 0;
  com->next->exclusive = exclusive;
  com->next->action = action;
  com->next->next = NULL;
  if( !( com->next->cmd = strdup( cmd ) ) ){
    fprintf( stderr, "%s: cannot allocate memory.\n", __FUNCTION__ );
    free( com->next );
    com->next = NULL;
    return -1;
  }
  return 0;
}

/* 現在までに受け付けたイベント列に該当するコマンドがあれば、対応するアクションを行う。 */
int jyp1_com_action(jyp1_t *jyp1, void *arg)
{
  static char ch;
  jyp1_com_t *cp;
  int ret = 0;

  ch = _jyp1_event( jyp1 );
  for( cp=jyp1->com.next; cp && cp->cmd; cp=cp->next ){
    if( cp->cmd[cp->progress] != ch ){
      cp->progress = 0;
      continue;
    }
    if( cp->cmd[++cp->progress] == JYP1_EVENT_NONE ){
      cp->progress = 0;
      ret = cp->action( arg );
      if( cp->exclusive ){
        _jyp1_com_clear( jyp1 );
        break;
      }
    }
  }
  return ret;
}

/* 登録されているコマンド一覧の表示(デバッグ用) */
void jyp1_com_print(jyp1_t *jyp1)
{
  jyp1_com_t *com;

  printf( "[registered commands]\n" );
  for( com=jyp1->com.next; com; com=com->next )
    printf( "%s: progress=%d, exclusive=%s, action=%p\n", com->cmd, com->progress, com->exclusive ? "true" : "false", com->action );
}

/* *** デバイス操作 *** */

/* デバイス情報がJY-P1Rに関するものか判別 */
static bool _jyp1_check_proc(char *buf)
{
  int bus, vendor, product, version;

  sscanf( buf, "I: Bus=%d Vendor=%d Product=%d Version=%d", &bus, &vendor, &product, &version );
  return vendor == JYP1_VENDOR_ID && product == JYP1_PRODUCT_ID;
}

/* JY-P1Rのデバイスハンドラ名を取得 */
static char *_jyp1_get_dev(char *buf, char *dev)
{
  char event[BUFSIZ];

  sscanf( buf, "H: Handlers=%s %s", event, dev );
  return dev;
}

/* JY-P1Rのデバイス情報が書かれているフィールドを探しハンドラ名を取得 */
static char *_jyp1_find_next_dev(FILE *fp, char dev[])
{
  char buf[BUFSIZ];

  while( !feof( fp ) ){
    if( !fgets( buf, BUFSIZ, fp ) ) break;
    if( buf[0] == 'I' ){
      if( !_jyp1_check_proc( buf ) ) continue;
      while( !feof( fp ) ){
        if( !fgets( buf, BUFSIZ, fp ) ) break;
        if( buf[0] == 'H' )
          return _jyp1_get_dev( buf, dev );
      }
    }
  }
  return NULL;
}

/* JY-P1Rのデバイスハンドラ名を検出 */
/* 複数個接続されているケースを想定していないので注意。 */
static char *_jyp1_find_dev(char *dev)
{
  FILE *fp;

  if( !( fp = fopen( PROCFILE, "r" ) ) ){
    perror( PROCFILE );
    return NULL;
  }
  dev = _jyp1_find_next_dev( fp, dev );
  fclose( fp );
  return dev;
}

/* JY-P1Rデバイスのopen */
int jyp1_open(jyp1_t *jyp1, const char *devfile)
{
  int count;
  struct js_event e;

  if( !( jyp1->fd = open( devfile, O_RDONLY ) ) ){
    fprintf( stderr, "cannot find device %s.\n", devfile );
    return -1;
  }
  /* JY-P1Rからの初期イベント群を読み飛ばす */
  for( count=0; count<JYP1_INIT_NUM; count++ )
    jyp1_read( jyp1, &e );
  sleep( 1 ); /* 念のため */
  /* コマンドリストの初期化 */
  _jyp1_com_init( jyp1 );
  return jyp1->fd;
}

/* JY-P1Rデバイスを自動検出してopen */
/* 複数個接続されているケースを想定していないので注意。 */
int jyp1_probe_open(jyp1_t *jyp1)
{
#define JSDEVNAME_BUFSIZ 10
  char devfile[BUFSIZ], dev[JSDEVNAME_BUFSIZ];

  if( !_jyp1_find_dev( dev ) ){
    fprintf( stderr, "JY-P1R not found.\n" );
    return -1;
  }
  sprintf( devfile, "/dev/input/%s", dev );
  return jyp1_open( jyp1, devfile );
}

/* JY-P1Rデバイスのclose */
int jyp1_close(jyp1_t *jyp1)
{
  _jyp1_com_destroy( jyp1 );
  return close( jyp1->fd );
}

/* JY-P1Rデバイスからのジョイスティックイベント読み込み */
/* 単独で使うことはほとんどない。 */
size_t jyp1_read(jyp1_t *jyp1, struct js_event *event)
{
  size_t size;

  if( ( size = read( jyp1->fd, event, sizeof(struct js_event) ) ) < 0 )
    fprintf( stderr, "ERRNO=%d: I/O violated.\n", errno );
  return size;
}
