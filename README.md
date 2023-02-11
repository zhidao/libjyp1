# サンワサプライのファミコン風ゲームパッドJY-P1RをLinuxで使おう
                                           by Zhidao
                                           2005. 9. 4. 作成
                                           2023. 2.10. 最終更新
---------------------------------------------------------------

## JY-P1Rとは

[ファミコン用ゲームパッドを真似たUSBゲームパッド](https://direct.sanwa.co.jp/ItemPage/JY-P1R)です。
ファミコンにはないD, Cボタンがあります。
機能的には今どきのジョイパッドに包含されるので、Linuxの標準ジョイスティック用デバイスドライバで認識できます。


## デバイスドライバとキャラクタデバイス作成

本文書を作成した2005年当時、最新版カーネルのバージョンは2.4.30で、USBジョイスティックを有効にするためには、デバイスドライバとキャラクタデバイスファイルが有効になるようカーネルをビルドする必要がありました。
2023年2月現在、筆者のOSはUbuntu22.04LTS、使用しているカーネルはバージョン5.15.0で、デバイスドライバまわりは何もしなくてもすぐに同ゲームパッドが使えるようです。
したがって本節の説明は無用かも知れませんが、記録として残しておきます。

まず、カーネルをビルドする際にはmenuconfigで

 - Input core support -> Joystick support
 - USB support -> USB Human Interface Device (full HID) support

の二箇所を有効にし、デバイスモジュールを作成します。
作成後は
```sh
% depmod -a
```
を忘れないようにしましょう。
この時点でUSBポートにJY-P1Rを差すとデバイスドライバinput、hidの二者がprobeされますが、これだけではまだ使えません。

デバイスドライバに対応するキャラクタデバイスファイルは/dev/input/js\*です。
これらが存在しない場合は、
```sh
% mknod /dev/input/js0 c 13 0
% mknod /dev/input/js1 c 13 0
% mknod /dev/input/js2 c 13 0
% mknod /dev/input/js3 c 13 0
```
として自分で作りましょう。
この辺は、カーネルソースのDocumentation/input/joystick.txtが参考になります。

デバイスファイルを作ったら
```sh
% modprobe joydev
```
します。これがデバイスドライバの本体です。

ジョイパッドと接続した直後のsyslogを見ると、
```
Vendor ID:773, Product ID:105
```
が読み取れます。
もしhotplugが動いているならば、この情報
```
joydev               0x0003 0x0773   0x0105    0x0000       0x0000    0x00         0x00            0x00            0x00            0x00            0x00            0x00000000
```
を/etc/hotplug/usb.handmapに一行追加しておくことで、接続時に自動的にjoydevをprobeしてくれるようにできます。


## パッド操作のためのテストプログラム

### 接続時の通信

パッド接続後の/proc/bus/input/devicesには、次の情報が足されています。
```
I: Bus=0003 Vendor=0773 Product=0105 Version=0110
N: Name="GreenAsia  USB Classic Gamepad "
P: Phys=usb-0000:02:00.0-2.2/input0
S: Sysfs=/devices/pci0000:00/0000:00:11.0/0000:02:00.0/usb2/2-2/2-2.2/2-2.2:1.0/0003:0773:0105.0003/i
nput/input8
U: Uniq=
H: Handlers=event6 js1 
B: PROP=0
B: EV=1b
B: KEY=3f00000000 0 0 0 0
B: ABS=3
B: MSC=10
```

Handlersの情報から、関連付けられているキャラクタデバイスは/dev/input/js1であると分かります。
試しに
```sh
% cat /dev/input/js1
```
などとすれば、パッド操作と連動してなにがしかの値がターミナルに流れるので、読めてることが分かります。
なお、キャラクタデバイスは他の機器の接続状況によって変わります。

ジョイスティックを用いるプログラミングには、
カーネルソースのDocumentation/input/joystick-api.txtが参考になります。
これを読みながら、まずは次のようなテストプログラムを作ってみました。
```C
#include <unistd.h>
#include <sys/fcntl.h>
#include <linux/joystick.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DEVFILE "/dev/input/js1"

int main(void)
{
  int fd;
  struct js_event e;
  size_t size;
  unsigned long ec = 0; /* event counter */

  if( !( fd = open( DEVFILE, O_RDONLY ) ) ){
    perror( DEVFILE );
    exit( 1 );
  }
  while( 1 ){
    if( ( size = read( fd, &e, sizeof(struct js_event) ) ) < 0 ){
      fprintf( stderr, "ERRNO=%d: I/O violated.\n", errno );
      exit( 1 );
    }
    printf( "[%ld] time=%d, value=%hd, type=%02x, id=%x\n", ec++, e.time, e.value, e.type, e.number );
  }
  close( fd );
  return 0;
}
```
DEVFILEは適宜書き換えて下さい。
これで、押されたボタンとイベント値との関連を調べることができます。

カーネル2.4系の上で上記プログラムを動かすと、最初に次のような29個のパケットが送られてきます。
```
[0] time=636877, value=0, type=81, id=0
[1] time=636877, value=0, type=81, id=1
[2] time=636877, value=0, type=81, id=2
[3] time=636877, value=0, type=81, id=3
[4] time=636877, value=0, type=81, id=4
[5] time=636878, value=0, type=81, id=5
[6] time=636878, value=0, type=81, id=6
[7] time=636878, value=0, type=81, id=7
[8] time=636878, value=0, type=81, id=8
[9] time=636878, value=0, type=81, id=9
[10] time=636878, value=0, type=81, id=a
[11] time=636878, value=1, type=81, id=b
[12] time=636878, value=0, type=81, id=c
[13] time=636878, value=0, type=81, id=d
[14] time=636878, value=0, type=81, id=e
[15] time=636878, value=0, type=81, id=f
[16] time=636878, value=0, type=81, id=10
[17] time=636878, value=0, type=81, id=11
[18] time=636878, value=0, type=81, id=12
[19] time=636878, value=0, type=81, id=13
[20] time=636878, value=0, type=81, id=14
[21] time=636878, value=0, type=81, id=15
[22] time=636878, value=0, type=81, id=16
[23] time=636878, value=0, type=81, id=17
[24] time=636878, value=0, type=82, id=0
[25] time=636878, value=0, type=82, id=1
[26] time=636878, value=-32767, type=82, id=2
[27] time=636878, value=-32767, type=82, id=3
[28] time=636878, value=-32767, type=82, id=4
```
type=81に対してidが0x0〜0x17、82に対してidが0〜4まで変化しています。
これらはどうやら、パッドから受け付ける全ての入力を走査しているようです。
なお、カーネル2.6以降は大幅に省略されて、次の8個になっています。
```
[0] time=722861, value=0, type=81, id=0
[1] time=722861, value=0, type=81, id=1
[2] time=722861, value=0, type=81, id=2
[3] time=722861, value=0, type=81, id=3
[4] time=722861, value=0, type=81, id=4
[5] time=722861, value=0, type=81, id=5
[6] time=722861, value=0, type=82, id=0
[7] time=722861, value=0, type=82, id=1
```

### パッド操作と受信データの対応

真面目にやるならば、前節の初期受信データに基づいて入力値のテーブルを自動作成するのが良いのでしょうが、今は決め打ちで、最初の29個ないし8個のパケットを読み捨てれば良い、と考えても構わないでしょう。
いずれにしても、その直後に入力待ち状態に入ります。
操作と受信データの対応を一つひとつ調べていくと、次の表が得られます。
```
           type   id  value
[十字左押]    2    0 -32767
[十字左離]    2    0  0
[十字右押]    2    0  32767
[十字右離]    2    0  0
[十字上押]    2    1 -32767
[十字上離]    2    1  0
[十字下押]    2    1  32767
[十字下離]    2    1  0
[A押]         1    0  1
[A離]         1    0  0
[B押]         1    1  1
[B離]         1    1  0
[C押]         1    2  1
[C離]         1    2  0
[D押]         1    3  1
[D離]         1    3  0
[START押]     1    4  1
[START離]     1    4  0
[SELECT押]    1    5  1
[SELECT離]    1    5  0
[TURBO押]     1    b  0
[TURBO離]     1    b  1
```

Documentation/input/joystick-api.txtを参考に上の結果を読めば、十字キーはドライバからはジョイスティックに見えていて、押すとその最大値が返ってくるということのようです（正負はスクリーン座標系に準じています）。
その他ボタンについては、基本的に押すと1、離すと0がそれぞれ返ってきますが、TURBOボタンだけはなぜか押/離と0/1の関係が逆になっています。
また、カーネル2.6系以降ではなぜかTURBOが読めなくなっています。

ここまで判れば一通りのことはできるようになります。

## キーコマンド→アクションを行うためのライブラリlibjyp1

JY-P1Rを便利に使えるライブラリを作ってみました。
```sh
% make
```
でライブラリファイルlibjyp1.soとサンプルプログラムsampleがコンパイルされます。

また、makefileの先頭にあるPREFIXを適当に変えた後に
```sh
% make && make install
```
とすれば、libjyp1.soとjyp1.hがインストールされます。

使い方はsample.cとjyp1.hを読んで頂ければおおよそお分かり頂けると思いますが、

 1. `jyp1_open()`または`jyp1_probe_open()`でデバイスファイルをオープンする。
 1. `jyp1_com_entry()`でコマンドに対するアクション(コールバック関数)を登録する。
 1. イベントループで`jyp1_com_action()`を呼ぶ。
 1. 終了時には`jyp1_close()`でデバイスファイルをクローズする。

という流れになります。
複数個のJY-P1Rを繋げられるように、`jyp1_t`という構造体を定義しています。
ただし、`jyp1_probe_open()`を使う場合は最初の1個目だけしか検出しませんのでご注意下さい。
サンプルプログラムは、コナミコマンドを打ち込むと終了します。
