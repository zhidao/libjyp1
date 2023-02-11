/* libjyp1: Linux用SANWA SUPPLY USBゲームパッドJY-P1操作ライブラリ
 *
 * 2005. 9. 4. Created. : Zhidao
 * 2023. 2.11. Last updated. : Zhidao
 */

#ifndef __JYP1_H__
#define __JYP1_H__

#include <unistd.h>
#include <sys/fcntl.h>
#include <linux/joystick.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

/* 入力イベントを管理するための構造体 */
typedef struct __jyp1_com_t{
  char *cmd;      /* 単独イベントを表すchar列 */
  int progress;   /* 複数イベントから成るコマンドの場合、現在までの入力が何個目のイベントまで消化しているかインジケートする */
  bool exclusive; /* trueならば排他的コマンドとして、コマンドを受け付けた際に他のコマンドprogressをクリアする。 */
  int (* action)(void*); /* コマンドに対応するアクション (コールバック関数) */
  struct __jyp1_com_t *next;
} jyp1_com_t;

/* ジョイパッド構造体 */
typedef struct{
  int fd; /* ファイルディスクリプタ */
  jyp1_com_t com; /* コマンドリスト */
} jyp1_t;

/* JY-P1Rデバイスをopenする。
 * 返り値 openしたキャラクタデバイスのファイルディスクリプタ
 * 失敗したら-1を返す。
 */
int jyp1_open(jyp1_t *jyp1, const char *devfile);

/* JY-P1Rデバイスを自動検出してopenする。
 * 複数個接続されているケースを想定していないので注意。
 * 返り値 openしたキャラクタデバイスのファイルディスクリプタ
 * 失敗したら-1を返す。
 */
int jyp1_probe_open(jyp1_t *jyp1);

/* JY-P1Rデバイスをcloseする。
 * 返り値はclose()に準じる。
 */
int jyp1_close(jyp1_t *jyp1);

/* JY-P1Rデバイスからのジョイスティックイベント読み込み */
size_t jyp1_read(jyp1_t *jyp1, struct js_event *event);

#define PROCFILE "/proc/bus/input/devices"

#define JYP1_VENDOR_ID  773
#define JYP1_PRODUCT_ID 105

/* ジョイパッドから受け付けられる単独イベントをcharで表したもの（文字の割り当ては適当） */
#define JYP1_EVENT_LEFT            '0'
#define JYP1_EVENT_RIGHT           '1'
#define JYP1_EVENT_HRZ_NEUTRAL     '2'
#define JYP1_EVENT_UP              '3'
#define JYP1_EVENT_DOWN            '4'
#define JYP1_EVENT_VRT_NEUTRAL     '5'
#define JYP1_EVENT_A_PRESSED       'A'
#define JYP1_EVENT_A_RELEASED      'a'
#define JYP1_EVENT_B_PRESSED       'B'
#define JYP1_EVENT_B_RELEASED      'b'
#define JYP1_EVENT_C_PRESSED       'C'
#define JYP1_EVENT_C_RELEASED      'c'
#define JYP1_EVENT_D_PRESSED       'D'
#define JYP1_EVENT_D_RELEASED      'd'
#define JYP1_EVENT_START_PRESSED   'S'
#define JYP1_EVENT_START_RELEASED  's'
#define JYP1_EVENT_SELECT_PRESSED  'E'
#define JYP1_EVENT_SELECT_RELEASED 'e'
#define JYP1_EVENT_TURBO_PRESSED   'T'
#define JYP1_EVENT_TURBO_RELEASED  't'
#define JYP1_EVENT_NONE            '_' /* terminater */

/* コマンドの登録。後から登録されたコマンドほど優先度が高い。 */
/* cmd: コマンドを表すchar列
 * exclusive: 排他的コマンドか否か
 * action: 対応するアクション(コールバック関数)
 */
int jyp1_com_entry(jyp1_t *jyp1, char *cmd, bool exclusive, int (*action)(void*));

/* 現在までに受け付けたイベント列に該当するコマンドがあれば、
 * 対応するアクションを行う。なければJY_ACTION_NONEを返す。 */
/* arg: コールバック関数に与えるためのユーティリティ引数。
 *      必要に応じて、適当にキャストして使うと良い。
 */
int jyp1_com_action(jyp1_t *jyp1, void *arg);

/* for debug */
void jyp1_com_print(jyp1_t *jyp1);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __JYP1_H__ */
