#include <stdio.h>
#include <allegro.h>

#define ulint unsigned long int
#define slint signed long int
#define usint unsigned short int
#define ssint signed short int

const ulint DEP_TIMER=1;
const ulint DEP_BUFFER=2;
const ulint DEP_KEYBOARD=2;
const int time_step=50;

ulint DEP;

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
		int assert(int,int);
		BITMAP*bitmap;
		unsigned long int*stos;
		unsigned long int*pola;
		unsigned short int*free_moves;
		int top;
		int x_s,y_s;
		unsigned long int closed_paths,opened_paths;
		//int a[7];//mozliwe ruchy w drugim ruchu
		unsigned long int moves;
		unsigned long int size;
};

FILE*zapis;
Tboard*board;
unsigned long int time_sec;


Tboard::Tboard(int n){
            //stos=new unsigned long int[n*n];
            pola=new unsigned long int[n*n];
            free_moves=new unsigned short int[n*n];
            int i,j;
            for (i=0;i<n*n-1;i++){/*stos[i]=0,*/pola[i]=0;};
            moves=closed_paths=opened_paths=top=0;
            size=n;
            x_s=y_s=20;
            for(i=1;i<=size;i++)
            for(j=1;j<=size;j++){
                free_moves[(i-1)*size+j-1]=0;
                if(check(i-1,j-2)==0)free_moves[(i-1)*size+j-1]++;
                if(check(i+1,j-2)==0)free_moves[(i-1)*size+j-1]++;
                if(check(i+2,j-1)==0)free_moves[(i-1)*size+j-1]++;
                if(check(i+2,j+1)==0)free_moves[(i-1)*size+j-1]++;
                if(check(i+1,j+2)==0)free_moves[(i-1)*size+j-1]++;
                if(check(i-1,j+2)==0)free_moves[(i-1)*size+j-1]++;
                if(check(i-2,j+1)==0)free_moves[(i-1)*size+j-1]++;
                if(check(i-2,j-1)==0)free_moves[(i-1)*size+j-1]++;
            };
                
            if((bitmap=create_video_bitmap(x_s*size,y_s*size))==NULL){
                alert("Nie uda³o siê zainicjowaæ bitmapy!!",NULL,NULL,"ok",NULL,1,2);
                DEP=DEP&!DEP_BUFFER;
            }else{
                DEP=DEP|DEP_BUFFER;
            };   
        };

inline int Tboard::check2(int x,int y){
    if((x>0)&&(x<=size)&&(y>0)&&(y<=size))
    return free_moves[(x-1)*size+y-1];
    else
    return -1;
};

inline int Tboard::check(int x,int y){
    if((x>0)&&(x<=size)&&(y>0)&&(y<=size))
    return pola[(x-1)*size+y-1];
    else
    return -1;
};


inline int Tboard::push(int x,int y,int i){
    //stos[top]=(x-1)*size+y-1;top++;
    pola[(x-1)*size+y-1]=i;
    if(y>1){
        if(check2(x-1,y-2)>0)free_moves[(x-2)*size+y-3]--;
        if(check2(x+1,y-2)>0)free_moves[(x-0)*size+y-3]--;
    };
    if(x<size-1){
        if(check2(x+2,y-1)>0)free_moves[(x+1)*size+y-2]--;
        if(check2(x+2,y+1)>0)free_moves[(x+1)*size+y-0]--;
    };
    if(y<size-1){
    	if(check2(x+1,y+2)>0)free_moves[(x-0)*size+y+1]--;
    	if(check2(x-1,y+2)>0)free_moves[(x-2)*size+y+1]--;
    };
    if(x>1){
        if(check2(x-2,y+1)>0)free_moves[(x-3)*size+y-0]--;
        if(check2(x-2,y-1)>0)free_moves[(x-3)*size+y-2]--;
    };
};

inline int Tboard::pop(int x,int y){
    //pola[stos[--top]]=0;
    pola[(x-1)*size+y-1]=0;
    if(y>1){
    if(check2(x-1,y-2)>-1)free_moves[(x-2)*size+y-3]++;
    if(check2(x+1,y-2)>-1)free_moves[(x-0)*size+y-3]++;
    };
	if(x<size-1){
    if(check2(x+2,y-1)>-1)free_moves[(x+1)*size+y-2]++;
    if(check2(x+2,y+1)>-1)free_moves[(x+1)*size+y-0]++;
    };
    if(y<size-1){
    if(check2(x+1,y+2)>-1)free_moves[(x-0)*size+y+1]++;
    if(check2(x-1,y+2)>-1)free_moves[(x-2)*size+y+1]++;
    };
    if(x>1){
    if(check2(x-2,y+1)>-1)free_moves[(x-3)*size+y-0]++;
    if(check2(x-2,y-1)>-1)free_moves[(x-3)*size+y-2]++;
    };
    //return stos[top];
};

void Tboard::draw_field(int x,int y,int back)
{
    //int x_s=10,y_s=10;
    
    if((x+y)%2==1){
        rectfill(bitmap, x_s*(x-1),y_s*(y-1),x_s*(x-1)+x_s,y_s*(y-1)+y_s,makecol24(128,128,255));
    }else{
        rectfill(bitmap,x_s*(x-1),y_s*(y-1),x_s*(x-1)+x_s,y_s*(y-1)+y_s,makecol24(0,0,255));
    };
    text_mode(makecol24(0,0,255));
    if(check(x,y))
    textprintf(bitmap,font,x_s*(x-1)+2,y_s*(y-1),makecol24(255,255,255),"%i ",check(x,y));
    else
    textprintf(bitmap,font,x_s*(x-1)+2,y_s*(y-1),makecol24(255,0,0),"%i ",check(x,y));
    if (check2(x,y)!=0)
    textprintf(bitmap,font,x_s*(x-1)+2,y_s*(y-1)+10,makecol24(0,255,0),"%i",free_moves[(x-1)*size+y-1]);
    else
    textprintf(bitmap,font,x_s*(x-1)+2,y_s*(y-1)+10,makecol24(255,0,0),"%i",free_moves[(x-1)*size+y-1]);
    text_mode(0);
};            
    
void Tboard::draw(int x,int y,int back)
{
    acquire_bitmap(bitmap);
    for(int i=1;i<=size;i++)
    for(int j=1;j<=size;j++)
    draw_field(i,j,back);
    release_bitmap(bitmap);
    blit(bitmap,screen,0,0,x,y,x_s*size,y_s*size);
    double moves_per_sec=double(moves)/double(time_sec)*(1000/double(time_step));
    textprintf(screen,font,x,y+y_s*size+10,makecol24(255,255,255),"%u",moves);
    textprintf(screen,font,x,y+y_s*size+20,makecol24(255,255,255),"ruchy na sekunde:%f",moves_per_sec);
//    textprintf(screen,font,x,y+y_s*size+10,makecol24(255,255,255),"%u:%i",moves,top);
};

/*int Tboard::check_knight(int x,int y)
{
 };*/

void timer_handle()
{
    time_sec++;
    board->draw(0,0,0);
};

inline int Tboard::assert(int x,int y)
{
    if(check(x-2,y-1)+check2(x-2,y-1)==0)return false;else
    if(check(x-2,y+1)+check2(x-2,y+1)==0)return false;else
    if(check(x-1,y+2)+check2(x-1,y+2)==0)return false;else
    if(check(x+1,y+2)+check2(x+1,y+2)==0)return false;else
    if(check(x+2,y+1)+check2(x+2,y+1)==0)return false;else
    if(check(x+2,y-1)+check2(x+2,y-1)==0)return false;else
    if(check(x-1,y-2)+check2(x-1,y-2)==0)return false;else
    if(check(x+1,y-2)+check2(x+1,y-2)==0)return false;else
    return true;

/* return !((check(x-2,y-1)+check2(x-2,y-1)==0)||(check(x-2,y+1)+check2(x-2,y+1)==0)||
 		(check(x-1,y+2)+check2(x-1,y+2)==0)||(check(x+1,y+2)+check2(x+1,y+2)==0)||
 		(check(x+2,y+1)+check2(x+2,y+1)==0)||(check(x+2,y-1)+check2(x+2,y-1)==0)||
 		(check(x-1,y-2)+check2(x-1,y-2)==0)||(check(x+1,y-2)+check2(x+1,y-2)==0));*/
};

int Tboard::move(int x,int y,int i)
{
    moves++;
    if (check(x,y)==0){
        push(x,y,i);
        if (i<size*size){
            if((i==size*size-1)||assert(x,y))
                if(!key[KEY_ESC]){
                        if(y<size-1){move(x-1,y+2,i+1);move(x+1,y+2,i+1);};
                        if(x<size-1){move(x+2,y+1,i+1);move(x+2,y-1,i+1);};
                        if(y>1){move(x+1,y-2,i+1);move(x-1,y-2,i+1);};
                        if(x>1){move(x-2,y-1,i+1);move(x-2,y+1,i+1);};
                };   
        }else{
                    //draw(200,0,0);
                    if((x==2&&y==3)||(x==3&&y==2)){
                        closed_paths++;
                        textprintf(screen,font,x_s*size,20,makecol24(0,255,0),"Tboard::move(%i,%i,%i):closed path=%i",x,y,i,closed_paths);
                        fprintf(zapis,"%i",closed_paths);
                        for(int x_=1;x_<=size;x_++)
                        {
                        fprintf(zapis,"|");
                        for(int y_=1;y_<=size;y_++)
                        fprintf(zapis,"%2u ",check(x_,y_));
                        fprintf(zapis,"|\n");
                        };
                        fprintf(zapis,"\n");
                    }else{
                        opened_paths++;
                        textprintf(screen,font,x_s*size,0,makecol24(0,255,0),"Tboard::move(%i,%i,%i):open path=%i",x,y,i,opened_paths);
                        /*for(int x_=1;x_<=size;x_++)
                        {
                        for(int y_=1;y_<=size;y_++)
                        fprintf(zapis,"%2u ",check(x_,y_));
                        fprintf(zapis,"\n");
                        };
                        fprintf(zapis,"\n");*/
                    };    
        };
        pop(x,y);
    };
};

void Tboard::chess()
{
    moves=0;
	int x=1,y=1;
	move(x,y,1);
};

void init()
{
    DEP=0;
    allegro_init();
    set_color_depth(24);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,480,0,0);
    set_display_switch_mode(SWITCH_BACKGROUND);
    if(install_keyboard()!=0){
        alert("Nie uda³o siê zainicjowaæ klawiatury",NULL,NULL,"OK",NULL,1,2);
    }else{
        DEP=DEP|DEP_KEYBOARD;
    };
    zapis=fopen("c:/knight.txt","w+");
    if(install_timer()!=0){
        alert("Nie uda³o siê zainicjowaæ timer'a",NULL,NULL,"OK",NULL,1,2);
    }else{
        DEP=DEP|DEP_TIMER;
    };
    text_mode(0);
};

void close()
{
    fclose(zapis);
};

int main(int argc, char *argv[])
{
    init();
    board=new Tboard(6);
    if(DEP|DEP_TIMER)
    install_int(timer_handle,time_step);
    time_sec=0;
    board->chess();
    if(DEP|DEP_TIMER)
    remove_int(timer_handle);
    close();
    clear_keybuf();
    while(!keypressed());
    return 0;
};    
      END_OF_MAIN()

