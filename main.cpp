#include <stdio.h>
#include <allegro.h>

#define ulint unsigned long int
#define slint signed long int
#define usint unsigned short int
#define ssint signed short int

ulint DEP_TIMER=1;
ulint DEP_BUFFER=2;
ulint DEP_KEYBOARD=2;
ulint DEP_FILE=4;
int time_step=200;
int  vert[8]={-2,-2,-1,+1,+2,+2,+1,-1};
int horiz[8]={-1,+1,+2,+2,+1,-1,-2,-2};
int  vert_[8]={-1,+1,+2,+2,+1,-1,-2,-2};
int horiz_[8]={-2,-2,-1,+1,+2,+2,+1,-1};
const unsigned long int red  =0x00FF0000;
const unsigned long int green=0x0000FF00;
const unsigned long int blue =0x000000FF;
const unsigned long int black=0x00000000;
const unsigned long int grey0=0x00000000;
const unsigned long int grey1=0x00111111;
const unsigned long int grey8=0x00888888;
const unsigned long int greyF=0x00FFFFFF;
const unsigned long int white=0x00FFFFFF;
//ulint DEP;
/******************************************************
 * Tboard                                             *
 * klasa                                              *
 *                                                    *
 * taka wirtualna szachwnica. Klasa znajduje          *
 * rozwi¹zania problemu skoczka, i wyœwietla aktualny *
 * postêp na keranie                                  *
 ******************************************************/
class Tdata
{
    int n;
    int x_s;
    int y_s;
    int timer_mode;
    int grafika;
    int ile;
    int zapis;
    int dump_data;
    int szachownica;
    int backtrace;
};
class Tboard
{
    public:
//konstruktor
        Tboard(int n);
        ~Tboard(){
            delete[] pola;
            //delete[] stos;
            destroy_bitmap(bitmap);
        };
        //pocz¹tek "gry"
		void chess();
	private:
	    //wykonanie pojedynczego ruchu, ewentualne wypisanie wyniku do pliku
	    int move(int x,int y,int i);
	    //sprawdza czy na danym polu skoczek ju¿ stawa³
	    int check(int x,int y);
	    int check2(int,int);
	    int push(int x,int y,int i);
		int  pop(int,int);
		void draw_field(int,int,int);
		void draw(int,int,int);
		int assert(int,int,int);
		void zapisz_stan();
		BITMAP*bitmap;
		unsigned long int*stos_x;
		unsigned long int*stos_y;
		unsigned long int*stos_k;
		unsigned long int*pola;
		unsigned short int*free_moves;
		int forced_move_x,forced_move_y;
		unsigned long int backtrace;
		int forced_num;
		int top;
		int x_s,y_s;
		int last_x,last_y;
		//int draw_x_start,draw_y_start,draw_x_end,draw_y_end;
		//int draw_x_size,draw_y_size;
		unsigned long int closed_paths,opened_paths;
		//int a[7];//mozliwe ruchy w drugim ruchu
		unsigned long int moves;
		unsigned long int size;
};

void write_debug(char*text){
    FILE*plik;
    plik=fopen("debug.txt","a");
    fprintf(plik,"%s",text);
    fclose(plik);
};
    
FILE*zapis;
Tboard*board;
Tdata data;
unsigned long int time_sec;
int debug=0;

/******************************************************
 * Tboard::Tboard(int n)                              *
 * konstruktor                                        *
 * n - rozmiar inicjowanej szachownicy(n*n)      *
 *                                                    *
 * Inicjuje szachownice, zmienne oraz bitmape         *
 ******************************************************/
Tboard::Tboard(int n){
            stos_x=new unsigned long int[n*n];
            stos_y=new unsigned long int[n*n];
            stos_k=new unsigned long int[n*n];
            pola=new unsigned long int[n*n];
            free_moves=new unsigned short int[n*n];
            int i,j,k;
            for (i=0;i<n*n-1;i++){/*stos[i]=0,*/pola[i]=0;};
            backtrace=forced_move_x=forced_move_y=moves=closed_paths=opened_paths=top=0;
            size=n;
            y_s=data.x_s;
            x_s=data.y_s;
            for(i=1;i<=size;i++)
            for(j=1;j<=size;j++){
                free_moves[(i-1)*size+j-1]=0;
                for(k=0;k<8;k++){
                    if(check(i+vert[k],j+horiz[k])==0)free_moves[(i-1)*size+j-1]++;
                };
            };
            free_moves[(size-3)*size+2-1]++;
            free_moves[(size-2)*size+3-1]++;
            if(data.grafika)
            if(data.szachownica)
            if((bitmap=create_video_bitmap(x_s*size,y_s*size))==NULL){
                alert("Nie uda³o siê zainicjowaæ bitmapy!!",NULL,NULL,"ok",NULL,1,2);
            };
            //DEP_BUFFER=0;
        };

/******************************************************
 * Tboard::check2(int x,int y)                        *
 * funkcja                                            *
 * x - wpó³rzêdna x na szachownicy                    *
 * y - wspó³rzêdna y na szachownicy                   *
 *                                                    *
 * Funckja zwraca ile jest mo¿liwych róchów z danego  *
 * pola szachownicy.                                  *
 ******************************************************/
inline int Tboard::check2(int x,int y){
    if((x>0)&&(x<=size)&&(y>0)&&(y<=size))
    return free_moves[(x-1)*size+y-1];
    else
    return -1;
};

/******************************************************
 * Tboard::check(int x,int y)                         *
 * funkcja                                            *
 * x - wpó³rzêdna x na szachownicy                    *
 * y - wspó³rzêdna y na szachownicy                   *
 *                                                    *
 * Funckja sprawdza czy skoczek stawa³ ju¿ na danym   *
 * polu szachownicy, i je¿eli tak to kiedy, a je¿eli  *
 * nie to zwraca 0.                                   *
 ******************************************************/
inline int Tboard::check(int x,int y){
    if((x>0)&&(x<=size)&&(y>0)&&(y<=size))
    return pola[(x-1)*size+y-1];
    else
    return -1;
};

/******************************************************
 * Tboard::push(int x,int y,int i)                    *
 * funkcja                                            *
 * x - wpó³rzêdna x na szachownicy                    *
 * y - wspó³rzêdna y na szachownicy                   *
 * i - numer ruchu                                    *
 *                                                    *
 * Funkcja dopisuje do aktualnego stanuy szachownicy  *
 * kolejny ruch na pole (x,y).                        *
 ******************************************************/
int Tboard::push(int x,int y,int i){
    int forced=0,ret=0,temp,k;
    forced_move_x=0;
    forced_move_y=0;
    if(check(x,y)==0){
    top++;
    stos_x[top]=x;
    stos_y[top]=y;
    pola[(x-1)*size+y-1]=i;
    for(k=0;k<8;k++){
        if(check2(x+vert[k],y+horiz[k])>0){
            --free_moves[(x+vert[k]-1)*size+y+horiz[k]-1];
            if(check2(x+vert[k],y+horiz[k])==1&&check(x+vert[k],y+horiz[k])==0){
            	forced++;forced_move_x=x+vert[k];forced_move_y=y+horiz[k];
           	};
       	};
    };
    if(data.grafika)
    if(data.szachownica==1)
    rect(screen,x_s*(x-1),y_s*(y-1),x_s*(x-1)+x_s-1,y_s*(y-1)+y_s-1,white);
    forced_num=forced;
    if (forced>1)ret=1;
    return ret;}else{return 2;};
};

/******************************************************
 * Tboard::pop(int x,int y)                           *
 * funkcja                                            *
 * x - wpó³rzêdna x na szachownicy                    *
 * y - wspó³rzêdna y na szachownicy                   *
 *                                                    *
 * Funkcja cofa skoczka o jeden ruch z pola (x,y).    *
 ******************************************************/
int Tboard::pop(int x,int y){

    pola[(x-1)*size+y-1]=0;
    for(int k=0;k<8;k++){
    	if(check2(x+vert[k],y+horiz[k])>-1)free_moves[(x+vert[k]-1)*size+y+horiz[k]-1]++;
   	};
    if(data.grafika)
    if(data.szachownica==1)
   	rect(screen,x_s*(x-1),y_s*(y-1),x_s*(x-1)+x_s-1,y_s*(y-1)+y_s-1,black);
   	if(top>0){
   	unsigned long int ret=stos_x[top]*(size-1)+stos_y[top]-1;
   	stos_x[top]=0;
   	stos_y[top]=0;
    top--;
    return ret;}else{
        return -1;
    };
};

void Tboard::draw_field(int x,int y,int back)
{
    switch(back)
    {
        case 0:rectfill(bitmap,x_s*(x-1),y_s*(y-1),x_s*(x-1)+x_s-2,y_s*(y-1)+y_s-2,red);break;
        case 1:rectfill(bitmap,x_s*(x-1),y_s*(y-1),x_s*(x-1)+x_s-2,y_s*(y-1)+y_s-2,blue);break;
        case 2:rectfill(bitmap,x_s*(x-1),y_s*(y-1),x_s*(x-1)+x_s-2,y_s*(y-1)+y_s-2,green);break;
        default:rectfill(bitmap,x_s*(x-1),y_s*(y-1),x_s*(x-1)+x_s-2,y_s*(y-1)+y_s-2,green);
        
    };
   //if(back>0)
   // rectfill(bitmap,x_s*(x-1),y_s*(y-1),x_s*(x-1)+x_s-2,y_s*(y-1)+y_s-2,green);
   // else
  //  rectfill(bitmap,x_s*(x-1),y_s*(y-1),x_s*(x-1)+x_s-2,y_s*(y-1)+y_s-2,red);
    
};

void Tboard::draw(int x,int y,int back)
{
    if(data.grafika){
    double moves_per_sec=double(moves)/double(time_sec)*(1000/double(time_step));
    double paths_per_sec=double(closed_paths)/double(time_sec)*(1000/double(time_step));
    int i,j;
    if(data.szachownica==2){
        acquire_bitmap(bitmap);
     for(i=1;i<=size;i++)
     for(j=1;j<=size;j++)
     draw_field(i,j,check(i,j));
     blit(bitmap,screen,0,0,0,0,x_s*size,y_s*size);
     
     for(i=1;i<=size;i++)
     for(j=1;j<=size;j++)
     draw_field(i,j,check2(i,j));
     blit(bitmap,screen,0,0,x_s*size+2,0,x_s*size,y_s*size);
     release_bitmap(bitmap);
 };
 if(data.szachownica){
    textprintf(screen,font,x,y+y_s*size+10,white,"%u",moves);
    textprintf(screen,font,x,y+y_s*size+20,white,"ruchy na sekunde:%f",moves_per_sec);
    textprintf(screen,font,x,y+y_s*size+30,white,"sciezki na sekunde:%f",paths_per_sec);
}else{
    textprintf(screen,font,0,10,white,"%u",moves);
    textprintf(screen,font,0,20,white,"ruchy na sekunde:%f",moves_per_sec);
    textprintf(screen,font,0,30,white,"sciezki na sekunde:%f",paths_per_sec);
};
    };
};

void timer_handle()
{
    time_sec++;
    board->draw(0,0,0);
};

inline int Tboard::assert(int ret,int x,int y)
{
    if(x==size-1&&y==3&&pola[size+2]!=0&&pola[size+2]!=(size*size))return false;else
    if(ret)return false;else
    return true;
};

void Tboard::zapisz_stan()
{
    if(data.zapis)
    fprintf(zapis,"%i\n",closed_paths);
    for(int x=1;x<=size;x++){
     fprintf(zapis,"|");
     for(int y=1;y<=size;y++)fprintf(zapis,"%3u ",check(x,y));
     fprintf(zapis,"|\n");
    };
};

int Tboard::move(int x,int y,int i)
{
    int ret;
    if(key[KEY_ESC]||closed_paths>data.ile)return 0;
    if(time_sec*time_step>data.backtrace&&closed_paths==0){time_sec=0;backtrace=size*size*1/4;};
    if(backtrace){if(i>backtrace){time_sec=0;return 0;}else backtrace=0;};
    if(i<size*size){
        ret=push(x,y,i);
        last_x=x;last_y=y;
        if ((i==size*size-1)||assert(ret,x,y))
        {
  		 moves++;
   		 if(forced_move_x)
   	 		move(forced_move_x,forced_move_y,i+1);
   	 	 else
      		  for(int k=0;k<8;k++)move(x+vert[k],y+horiz[k],i+1);
   		};
        if(ret!=2)pop(x,y);
    };
    if(i==size*size){
                    if((x==size-1&&y==3)||(x==size-2&&y==2)){
                        zapisz_stan();
                        closed_paths++;
                        if(data.grafika)
                        if(data.szachownica)
                        textprintf(screen,font,0,y_s*size+60,makecol24(0,255,0),"Tboard::move(%i,%i,%i):closed path=%i",x,y,i,closed_paths);
                        else
                        textprintf(screen,font,0,60,makecol24(0,255,0),"Tboard::move(%i,%i,%i):closed path=%i",x,y,i,closed_paths);
                    }else{
                        opened_paths++;
                        //if(data.grafika)
                        //textprintf(screen,font,x_s*size,0,makecol24(0,255,0),"Tboard::move(%i,%i,%i):open path=%i",x,y,i,opened_paths);
                    };
    };
};

void Tboard::chess()
{
    moves=0;
	int x=size,y=1;
	move(x,y,1);
};

void init()
{
    if(data.grafika)
    {
     set_color_depth(24);
     set_gfx_mode(GFX_AUTODETECT_WINDOWED,1000,700,0,0);
     set_display_switch_mode(SWITCH_BACKGROUND);
    };
    if(install_keyboard()!=0){
        alert("Nie uda³o siê zainicjowaæ klawiatury",NULL,NULL,"OK",NULL,1,2);
    };
    if(data.zapis)zapis=fopen("c:/knight.txt","w+");
    if(install_timer()!=0){
        alert("Nie uda³o siê zainicjowaæ timer'a",NULL,NULL,"OK",NULL,1,2);
    };
    text_mode(0);
};

void close()
{
    if(DEP_FILE)
    fclose(zapis);
};

void load_data(){
    set_config_file("config.cfg");
    data.n=get_config_int(NULL,"n",8);
    data.x_s=get_config_int("grafika","szerokosc_plytki",10);
    data.y_s=get_config_int("grafika","wysokosc_plytki",10);
    data.timer_mode=get_config_int("grafika","timer",0);
    data.grafika=get_config_int("grafika","tryb_graficzny",0);
    data.ile=get_config_int(NULL,"ile",0);
    data.zapis=get_config_int("zapis","zapis",1);
    data.dump_data=get_config_int("debug","dump_data",0);
    data.szachownica=get_config_int("grafika","szachownica",0);
    data.backtrace=get_config_int(NULL,"backtrace_time",500);
};
    
int main(int argc, char *argv[])
{
    allegro_init();
    load_data();
    init();
    board=new Tboard(data.n);
    if(data.timer_mode)
     install_int(timer_handle,time_step);
    time_sec=0;
    board->chess();
    if(data.timer_mode)
     remove_int(timer_handle);
    close();
    clear_keybuf();
    while(!keypressed());
    return 0;
};
      END_OF_MAIN()

