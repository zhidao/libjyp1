#include <jyp1.h>

static char conami_command_str[] = {
  JYP1_EVENT_UP, JYP1_EVENT_VRT_NEUTRAL, JYP1_EVENT_UP, JYP1_EVENT_VRT_NEUTRAL,
  JYP1_EVENT_DOWN, JYP1_EVENT_VRT_NEUTRAL, JYP1_EVENT_DOWN, JYP1_EVENT_VRT_NEUTRAL,
  JYP1_EVENT_LEFT, JYP1_EVENT_HRZ_NEUTRAL, JYP1_EVENT_RIGHT, JYP1_EVENT_HRZ_NEUTRAL,
  JYP1_EVENT_LEFT, JYP1_EVENT_HRZ_NEUTRAL, JYP1_EVENT_RIGHT, JYP1_EVENT_HRZ_NEUTRAL,
  JYP1_EVENT_B_PRESSED, JYP1_EVENT_B_RELEASED, JYP1_EVENT_A_PRESSED, JYP1_EVENT_A_RELEASED, JYP1_EVENT_NONE
};

int left_pressed(void *arg){ printf( "left pressed.\n" ); return 0; }
int right_pressed(void *arg){ printf( "right pressed.\n" ); return 0; }
int hrz_neutral(void *arg){ printf( "left-right neutral.\n" ); return 0; }
int up_pressed(void *arg){ printf( "up pressed.\n" ); return 0; }
int down_pressed(void *arg){ printf( "down pressed.\n" ); return 0; }
int vrt_neutral(void *arg){ printf( "up-down neutral.\n" ); return 0; }
int a_pressed(void *arg){ printf( "A pressed.\n" ); return 0; }
int a_released(void *arg){ printf( "A released.\n" ); return 0; }
int b_pressed(void *arg){ printf( "B pressed.\n" ); return 0; }
int b_released(void *arg){ printf( "B released.\n" ); return 0; }
int c_pressed(void *arg){ printf( "C pressed.\n" ); return 0; }
int c_released(void *arg){ printf( "C released.\n" ); return 0; }
int d_pressed(void *arg){ printf( "D pressed.\n" ); return 0; }
int d_released(void *arg){ printf( "D released.\n" ); return 0; }
int start_pressed(void *arg){ printf( "start button pressed.\n" ); return 0; }
int start_released(void *arg){ printf( "start button released.\n" ); return 0; }
int select_pressed(void *arg){ printf( "select button pressed.\n" ); return 0; }
int select_released(void *arg){ printf( "select button released.\n" ); return 0; }
int hadouken(void *arg){ printf( ">> hadouken!\n" ); return 0; }
int shouryuuken(void *arg){ printf( ">> shouryuuken!\n" ); return 0; }
int conami_command(void *arg){ printf( "conami command enterted!\n" ); return -1; }

int main(int argc, char *argv[])
{
  jyp1_t jyp1;

  if( argc == 1 ){
    if( jyp1_probe_open( &jyp1 ) < 0 ) return EXIT_FAILURE;
  } else{
    if( jyp1_open( &jyp1, argv[1] ) < 0 ) return EXIT_FAILURE;
  }

  jyp1_com_entry( &jyp1, "0_", 0, left_pressed );
  jyp1_com_entry( &jyp1, "1_", 0, right_pressed );
  jyp1_com_entry( &jyp1, "2_", 0, hrz_neutral );
  jyp1_com_entry( &jyp1, "3_", 0, up_pressed );
  jyp1_com_entry( &jyp1, "4_", 0, down_pressed );
  jyp1_com_entry( &jyp1, "5_", 0, vrt_neutral );
  jyp1_com_entry( &jyp1, "S_", 0, start_pressed );
  jyp1_com_entry( &jyp1, "s_", 0, start_released );
  jyp1_com_entry( &jyp1, "E_", 0, select_pressed );
  jyp1_com_entry( &jyp1, "e_", 0, select_released );
  jyp1_com_entry( &jyp1, "A_", 0, a_pressed );
  jyp1_com_entry( &jyp1, "a_", 0, a_released );
  jyp1_com_entry( &jyp1, "B_", 0, b_pressed );
  jyp1_com_entry( &jyp1, "b_", 0, b_released );
  jyp1_com_entry( &jyp1, "C_", 0, c_pressed );
  jyp1_com_entry( &jyp1, "c_", 0, c_released );
  jyp1_com_entry( &jyp1, "D_", 0, d_pressed );
  jyp1_com_entry( &jyp1, "d_", 0, d_released );
  jyp1_com_entry( &jyp1, "415A_", 1, hadouken );
  jyp1_com_entry( &jyp1, "1241A_", 1, shouryuuken );
  jyp1_com_entry( &jyp1, conami_command_str, 1, conami_command );
  jyp1_com_print( &jyp1 );
  fprintf( stderr, "ready.\n" );

  while( jyp1_com_action( &jyp1, NULL ) != -1 );

  fprintf( stderr, "terminating..." );
  jyp1_close( &jyp1 );
  fprintf( stderr, "done.\n" );
  return EXIT_SUCCESS;
}
