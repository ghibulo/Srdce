#include <stdio.h>
#include <allegro5/allegro.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>

#define PI 3.14159265

const float FPS = 60;
const int SCREEN_W = 1024;
const int SCREEN_H = 768;
const int BOUNCER_SIZE = 2;
const int BOUNCER_MARGIN = BOUNCER_SIZE+0;
const int COLOUR_LIMIT = 220;
const int ORIENTATIONLIMIT = 45;
const int DISTANCELIMIT = 20; //vzdalenost v bouncerech, ktera jeste bude zanedbana

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_BITMAP *bouncer = NULL;
ALLEGRO_BITMAP *center = NULL;

using namespace std;


typedef vector<int> RADKA;
vector<RADKA> mapa;

vector<RADKA> calcul;

void init(void) {

    if(!al_init()) {
      fprintf(stderr, "failed to initialize allegro!\n");
      exit(-1);
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display) {
      fprintf(stderr, "failed to create display!\n");
      exit(-1);
    }

    bouncer = al_create_bitmap(BOUNCER_SIZE, BOUNCER_SIZE);
    center = al_create_bitmap(BOUNCER_SIZE, BOUNCER_SIZE);
    if((!bouncer)||(!center)) {
      fprintf(stderr, "failed to create bouncer bitmap!\n");
      al_destroy_display(display);
      exit(-1);
    }
    al_set_target_bitmap(bouncer);
    al_clear_to_color(al_map_rgb(0,0,0));
    al_set_target_bitmap(center);
    al_clear_to_color(al_map_rgb(255,0,0));

    al_set_target_bitmap(al_get_backbuffer(display));

    event_queue = al_create_event_queue();
    if(!event_queue) {
      fprintf(stderr, "failed to create event_queue!\n");
      al_destroy_bitmap(bouncer);
      al_destroy_display(display);
      exit(-1);

    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_clear_to_color(al_map_rgb(255,255,255));
    al_flip_display();
}

vector<int> split(string str, char delimiter) {
  vector<int> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;

  while(getline(ss, tok, delimiter)) {
    internal.push_back(stoi(tok));
  }

  return internal;
}

void openCSV(string name) {
    string line;
    ifstream myfile (name);
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
          RADKA sep = split(line, ',');
          mapa.push_back(sep);
          //init calcul (o kazdem bodu pocet smeru)
          RADKA norient;
          for(int i=0;i<sep.size();i++)
            norient.push_back(0);
          calcul.push_back(norient);
        }
        myfile.close();
    } else cout << "Unable to open file";

}

void printMap(void) {
    for (int i=0;i<mapa.size();i++) {
        for (int j=0;j<mapa[0].size();j++) {
            cout << mapa[i][j] << " - ";
      }
      cout << endl;
    }
}

bool isCoordInMap (int x, int y) {
    return (x>=0)&&(x<mapa.size())&&(y>=0)&&(y<mapa[0].size());
}

bool isTheSameOrientation(int alfa, int beta) {
    return ((abs(alfa-beta)<ORIENTATIONLIMIT)||(abs(alfa+180-beta)<ORIENTATIONLIMIT));
}

int getDistance(int x, int y, int orientation) {
    float vy = sin(orientation*PI/180);
    float vx = cos(orientation*PI/180);
    int dist = 1;
    int px, py;
    do {
        px = x + vx*dist;
        py = y + vy*dist;
        dist++;
    } while (isCoordInMap(px,py)&&(mapa[px][py]<COLOUR_LIMIT));
    return --dist;
}

int doCalculPoint(int x, int y) { //hodne pocitanych veci jen pro ladeni
    int dist, maxdist;
    int maxorient[4] = {-1,-1,-1,-1}; //smery s max.vzdalenosti
    int maxdistkv[4] = {0,0,0,0}; //maximalni vzdalenosti ve ctyrech smerech
    for (int kvadrant=0;kvadrant<4;kvadrant++) {
        maxdist=0;
        for(int i=0;i<90;i++) {
            dist = getDistance(x,y,i+kvadrant*90);
            if (dist>maxdist) {
                        maxdist = dist;
                        maxorient[kvadrant] = i+kvadrant*90;
                        maxdistkv[kvadrant]=maxdist;
            }
        }
    }
    //vyfiltrujeme v podstate stejne smery
    for (int i=0;i<4;i++) {
        if (maxorient[i]>=0) {
            int nasl = (i+1)%4; //vedlejsi kvadrant
            int opacny = (i+2)%4; //opacny kvadrant
            for (int j=0;j<2;j++) {
                int with = (j==0)?nasl:opacny;
                if ((maxorient[with]>=0)&&isTheSameOrientation(maxorient[i],maxorient[with])) {
                    if (maxdistkv[i]>maxdistkv[with]) {
                        maxorient[with]=-1;maxdistkv[with]=0;
                    } else {
                        maxorient[i]=-1;maxdistkv[i]=0;
                    }
                }
            }
        }
    }
    //spocitam pocet smeru
    int vysledek=0;
    for(int i=0;i<4;i++) if (maxdistkv[i]>DISTANCELIMIT) vysledek++;
    calcul[x][y] = vysledek;
    return vysledek;
}

void doCalcul() {
    for (int i=0;i<mapa.size();i++) {
        for (int j=0;j<mapa[0].size();j++) {
            if (mapa[i][j]<COLOUR_LIMIT) {
                doCalculPoint(i,j);
            }
      }
    }


}



int main(int argc, char **argv){


    init();

    openCSV(argv[1]);
    //printMap();
    doCalcul();







    //al_start_timer(timer);
    while (1) {
        ALLEGRO_EVENT ev;
        ALLEGRO_TIMEOUT timeout;
        al_init_timeout(&timeout, 0.06);

        bool get_event = al_wait_for_event_until(event_queue, &ev, &timeout);

        if(get_event && ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }

        al_clear_to_color(al_map_rgb(255,255,255));
        for (int bx=0;bx<mapa.size();bx++) {
            for (int by=0;by<mapa[0].size();by++) {
                if (mapa[bx][by]<COLOUR_LIMIT) {
                    if (calcul[bx][by]>=2)
                        al_draw_bitmap(center,  bx*BOUNCER_MARGIN, by*BOUNCER_MARGIN, 0);
                    else
                        al_draw_bitmap(bouncer,  bx*BOUNCER_MARGIN, by*BOUNCER_MARGIN, 0);
                }
            }
        }
        al_flip_display();
    }

    al_destroy_bitmap(bouncer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}

