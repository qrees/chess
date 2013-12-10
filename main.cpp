#include <stdio.h>
#include <allegro.h>

#define ulint unsigned long int
#define slint signed long int
#define usint unsigned short int
#define ssint signed short int
#define debuguj 1

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

void debug(char*text)
{
    #ifdef debuguj
    FILE*plik;
    plik=fopen("debug.txt","a+");
    fprintf(plik,"%s ",text);
    fclose(plik);
    #endif
};

void debug(int liczba)
{
    #ifdef debuguj
    FILE*plik;
    plik=fopen("debug.txt","a+");
    fprintf(plik,"%i ",liczba);
    fclose(plik);
    #endif
};

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
	    unsigned long int arraypos(int x,int y);
	    int push(int,int,int);
		int pop();
		void draw_field(int,int,int);
		void draw(int,int,int);
		int assert(int,int,int);
		void zapisz_stan();
		BITMAP*bitmap;
		long int*stos_x;
		long int*stos_y;
		long int*stos_k;
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

    
FILE*zapis;
Tboard*board;
Tdata data;
unsigned long int time_sec;
//int debug=0;

/******************************************************
 * Tboard::Tboard(int n)                              *
 * konstruktor                                        *
 * n - rozmiar inicjowanej szachownicy(n*n)      *
 *                                                    *
 * Inicjuje szachownice, zmienne oraz bitmape         *
 ******************************************************/
Tboard::Tboard(int n){
            stos_x=new long int[n*n];
            stos_y=new long int[n*n];
            stos_k=new long int[n*n];
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

inline unsigned long int Tboard::arraypos(int x,int y){
    return (x-1)*size+y-1;
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
    if(check(x,y)==0){
        top++;
        stos_x[top]=x;stos_y[top]=y;stos_k[top]=i;
        pola[arraypos(x,y)]=top;
        forced_move_x=forced_move_y=0;
        int forced;
        for(int k=0;k<8;k++){
        if(check2(x+vert[k],y+horiz[k])>0){
            --free_moves[(x+vert[k]-1)*size+y+horiz[k]-1];
            if(check2(x+vert[k],y+horiz[k])==1&&check(x+vert[k],y+horiz[k])==0){
            	forced++;forced_move_x=x+vert[k];forced_move_y=y+horiz[k];
           	};
       	};
    };
    
        rect(screen,x*x_s,y*y_s,x*x_s+x_s-1,y*y_s+y_s-1,white);
    };
};

/******************************************************
 * Tboard::pop(int x,int y)                           *
 * funkcja                                            *
 * x - wpó³rzêdna x na szachownicy                    *
 * y - wspó³rzêdna y na szachownicy                   *
 *                                                    *
 * Funkcja cofa skoczka o jeden ruch z pola (x,y).    *
 ******************************************************/
int Tboard::pop(){
    pola[arraypos(stos_x[top],stos_y[top])]=0;
    
    if(data.grafika)
    if(data.szachownica==1)
   	rect(screen,x_s*(stos_x[top]),      y_s*(stos_y[top]),
                x_s*(stos_x[top])+x_s-1,y_s*(stos_y[top])+y_s-1,black);
   	if(top>0){
   	stos_x[top]=0;
   	stos_y[top]=0;
    top--;
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
    //board->draw(0,0,0);
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
    while(top<size*size&&(key[KEY_ESC]==0)){
        if(stos_k[top]<8)
        if(forced_move_x)
          push(forced_move_x,forced_move_y,-1);
        else
          push(stos_x[top]+vert[stos_k[top]],stos_y[top]+horiz[stos_k[top]],-1);
        else
          pop();
        debug("move");debug(stos_x[top]);debug(stos_y[top]);debug(stos_k[top]);debug("\n");
        clear_keybuf();
        while(!keypressed());
        stos_k[top]++;
    };
};

void Tboard::chess()
{
    moves=0;
	push(1,1,0);
	move(1,1,1);
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

