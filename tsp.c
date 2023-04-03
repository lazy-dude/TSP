// Originally from:
// https://github.com/DubiousCactus/GeneticAlgorithm

// TODO call as compiler flag:
//#define NDEBUG
//#define EXAMPLE_8
//#define EXAMPLE_50

#define MAX_STATES 64 // 1000 25 15 10000 50 102

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

//#include <simple2d.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
const char *title = "TSP problem , graph theory";
#define FONT "/usr/share/fonts/gnu-free/FreeSans.ttf"
//#define FONT "/usr/local/share/agar/fonts/monoalgue.ttf"
const SDL_Color bg={255, 255, 255,255};//{32, 32, 32, 0xFF}; // background
const SDL_Color pol={200, 200, 200, 0xFF}; // polygon
const SDL_Color white = {255, 255, 255,255};
const SDL_Color black = {0, 0, 0,255};
const SDL_Color blue={0, 0, 255, 255};
const SDL_Color green ={0,255,0, 255};
#define LINE_COLOR 0xFF0FFFF0
#define POINT_SIZE 512
//#define X_MAX 1 
//#define Y_MAX 1
//#define CITY_NUM 2


#ifdef EXAMPLE_8
#define X_MAX 800 
#define Y_MAX 600 
#define CITY_NUM 8 // TODO compute it , many random cities.
#endif

#ifdef EXAMPLE_50
#define X_MAX 900//1000 
#define Y_MAX 700//1000
#define CITY_NUM 50
#endif

#define ALL_VISITED -1
#define NOT_OC 0 //-1
#define LAST_LAYER -1
//#define MAX_EDGES CITY_NUM
#define MAX_CYCLES 30
#define NO_CITY -1

static int last_i = 0;

enum RW
{
    READ = 0,
    WRITE
}; // TODO add FREE
const int sectors = 500;
float radius = 5.0;
struct city
{
    char *name;
    float x;
    float y;
    // visited
    bool visited;
    int on_cycle; // TODO polygon instead of cycle
    // struct city *next;
    int joint; // to an outer? (inner) cycle
    int next_i;
};
typedef struct city city;

// ************************************************ //

//void display_text(char *in_text, float x, float y);
void render_text(SDL_Renderer ** renderer,TTF_Font *font,Sint16 x, Sint16 y, char * text);

bool correct_index(int i)
{
    if(i == ALL_VISITED || (i >= 0 && i < CITY_NUM))
        return true;

    return false;
}

void low_dist(float *dis, enum RW rw)
{
    static float md = FLT_MAX;
    
    if(rw == READ)
        *dis = md;
    else // WRITE
        if(*dis<md)
            md=*dis;
    return;
}

void distance_keeper(float *dis, enum RW rw)
{
    static float kdis = 0;

    if(rw == READ)
        *dis = kdis;
    else // WRITE
        kdis += *dis;
    return;
}
void city_info(city *out_c, int cnum, enum RW rw) 
{
    
    assert(correct_index(cnum));
    static city cities[CITY_NUM] = {
        
        {.x = X_MAX * 0.47,
         .y = Y_MAX * 0.03,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Lille"}, // Beginning city
        {.x = X_MAX * 0.51,
         .y = Y_MAX * 0.15,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Paris"},
        {.x = X_MAX * 0.8,
         .y = Y_MAX * 0.06,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Reims"},
        {.x = X_MAX * 0.7,
         .y = Y_MAX * 0.65,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Lyon"},
        {.x = X_MAX * 0.9,
         .y = Y_MAX * 0.7,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Marseille"},
        {.x = X_MAX * 0.15,
         .y = Y_MAX * 0.3,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Nantes"},
        {.x = X_MAX * 0.05,
         .y = Y_MAX * 0.42,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "La Rochelle"},
        {.x = X_MAX * 0.2,
         .y = Y_MAX * 0.59,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Bordeaux"},
        
    }; 
    
    if(rw == READ)
    {
        *out_c = cities[cnum];
    }
    else // WRITE
        cities[cnum] = *out_c;
    return;
    
}

float distance(city from, city to)
{
    float dis = 0.0;
    float dx = fabsf(to.x - from.x);
    float dy = fabsf(to.y - from.y);

    dis = sqrt(pow(dx, 2) + pow(dy, 2));

    return dis;
}
bool almost_all_visited(void)
{
    int i;
    city ci;
    int visited_cnt = 0;
    for(i = 0; i < CITY_NUM; i++)
    {
        city_info(&ci, i, READ);
        if(ci.visited == true) 
        {
            visited_cnt++;
            
        }
    }
    
    if(visited_cnt >= CITY_NUM) 
        return true;
    return false;
}

void cycle_keeper(int *vertex, int vertex_ind, int no_cycles, enum RW rw)
{
    
    assert(vertex_ind >= 0);
    assert(vertex_ind < CITY_NUM);
    assert(no_cycles >= 0);
    assert(no_cycles <= MAX_CYCLES);
    static int flag = 0;
    static int *vertices_arr;
    vertex_ind++;

    if(flag == 0)
    {
        vertices_arr = calloc((CITY_NUM) * (MAX_CYCLES), sizeof(int)); 
        
        flag = 1;
    }

    int index = vertex_ind * MAX_CYCLES + no_cycles;
    if(rw == READ)
        *vertex = vertices_arr[index]; 
    else // WRITE
        vertices_arr[index] = *vertex;
    // TODO FREE

}

// ************************************************ //

int nearest(int since) 
{

    city out_c_s;
    
    assert(correct_index(since));
    city_info(&out_c_s, since, READ);
    
    int save_since = since;

    while(out_c_s.visited == true && since < CITY_NUM)
    {
        since++;
        city_info(&out_c_s, since, READ);
    }
    
    int i, save_i = 0;
    float dis = X_MAX + Y_MAX;
    city out_c;

    for(i = 0; i < CITY_NUM; i++) 
    {
        city_info(&out_c, i, READ);
        if(out_c.visited == true || i == since)
        {
            continue;
        }

        float curr_dis = distance(out_c_s, out_c);
        if(curr_dis < dis)
        {
            dis = curr_dis;
            save_i = i;
        }
    }

    assert(correct_index(save_i));
    city_info(&out_c, save_i, READ);

    out_c_s.visited = true;
    out_c_s.next_i = save_i; 
    assert(correct_index(since));
    city_info(&out_c_s, since, WRITE);

    if(out_c.visited == false && save_i != since && save_i != 0)
    {
        assert(correct_index(save_i));

        return save_i;
    }
    else
    {
        for(i = 1; i < CITY_NUM; i++)
        {
            city_info(&out_c, i, READ);
            if(out_c.visited == false && i != since)
            {
                assert(correct_index(i));
                out_c_s.next_i = i; 
                return i;
            }
        }
    }

    city_info(&out_c, save_since, READ);
    out_c.visited = false;
    out_c.next_i = ALL_VISITED; 
    city_info(&out_c, save_since, WRITE);
    return ALL_VISITED; 
}

// total distance for all cities visited
void produce_ways(void) 
{
    int nolines = 1; 
    int next = 0;

    int i = 0;

    for(; nolines < CITY_NUM + 1; nolines++, i = next)

    {
        assert(i >= ALL_VISITED && i < CITY_NUM);

        city cn, cn2;
        
        next = nearest(i); 
        assert(next >= ALL_VISITED && next < CITY_NUM);
        if(next == ALL_VISITED || next == 0) 
            break;

        assert(i >= ALL_VISITED && i < CITY_NUM);
        city_info(&cn, i, READ);
        cn.visited = true;
        city_info(&cn, i, WRITE);

        assert(next >= ALL_VISITED && next < CITY_NUM);
        city_info(&cn2, next, READ);

        cn.next_i = next; 

        float dis = distance(cn, cn2);
        distance_keeper(&dis, WRITE);

        // TODO m add this again
        //display_text("t1 inside loop", 25, 0.8 * Y_MAX);
        
    }
    return;
}

void last_draw(void) 
{
    city ci;
    int i;
    for(i = 1; i < CITY_NUM; i++)
    {
        city_info(&ci, i, READ);
        
        if(ci.next_i == ALL_VISITED) 
            break;
    } 
    
    if(i == CITY_NUM || i == ALL_VISITED)
        return; 
    
    assert(correct_index(i));
    city_info(&ci, i, READ);

    city cn;
    city_info(&cn, 0, READ); 

    float pre_dis;
    distance_keeper(&pre_dis, READ);
    float dis = distance(ci, cn);
    distance_keeper(&dis, WRITE);

    last_i = i;
    return;
}

// ************************************************ //
void print_vertices(int const *vertices, int vert_num);

int vertex(float x, float y)
{
    //static bool prev[CITY_NUM+1]={false};
    //static int pi=0;
    int i;
    int cnt = 0;
    for(i = 0; i < CITY_NUM; i++)
    {
        city ci;
        city_info(&ci, i, READ);
        //prev[i]=false;
        if(ci.on_cycle == NOT_OC)
        {
            //prev[i]=true;
            cnt++;
        }
    }
    if(cnt < 3)
        return LAST_LAYER; 

    //city csi;
    city eg = {.x = x, .y = y};
    int save_i = 0;
    float min_dis = X_MAX + Y_MAX;
    city ci; 
    for(i = 0; i < CITY_NUM; i++)
    {
        city_info(&ci, i, READ);
        float dis = distance(ci, eg);
        if(dis < min_dis && ci.on_cycle == NOT_OC )//&& prev[i]==true) 
        {
            min_dis = dis;
            save_i = i;
            //city_info(&csi, save_i, READ);
        }
    }
    
    
    //csi.on_cycle=0;
    //city_info(&csi, save_i, WRITE);
    
    assert(correct_index(save_i));
    return save_i;
}

bool vertex3(int *v3)
{
    
    v3[0] = vertex(X_MAX /2.0 , 0.0);
    v3[1] = vertex(0.0, Y_MAX);
    v3[2] = vertex(X_MAX, Y_MAX);
    //assert(v3[0]!=v3[1] || v3[0]==LAST_LAYER || v3[1]==LAST_LAYER);
    //assert(v3[1]!=v3[2] || v3[1]==LAST_LAYER || v3[2]==LAST_LAYER);
    //assert(v3[0]!=v3[2] || v3[0]==LAST_LAYER || v3[2]==LAST_LAYER);
    
    if(v3[0] == LAST_LAYER || v3[1] == LAST_LAYER || v3[2] == LAST_LAYER)
        return false;
    if(v3[0]==v3[1]||v3[0]==v3[2]||v3[1]==v3[2])
        return false;
    
    return true;
}

#define MIN_DIFF 5
#define EPS 0.01
bool same_coor(city c1, city c2)
{
    if(fabs(c2.x-c1.x)<EPS && fabs(c2.y-c1.y)<EPS)
        return true;
    return false;
}
bool lines_cross(city c1, city c2, city c3, city c4) // TODO check the internet , lines segment
{
    //assert(c1.x != c3.x || c1.y != c3.y); // TODO more like this  asserts
    assert(strcmp(c1.name, c2.name));
    assert(strcmp(c3.name, c4.name));
    
    assert(fabs(c2.x-c1.x) >EPS || fabs(c2.y-c1.y) >EPS);
    assert(fabs(c4.x-c3.x) >EPS || fabs(c4.y-c3.y) >EPS);
    if(same_coor(c2,c3) && same_coor(c1,c4))
    {
        fprintf(stderr,"same segments\n");
        exit(1);
    }
    if(same_coor(c2,c3))
        return false;
    
    
    float a = (c2.y - c1.y) / (c2.x - c1.x);
    float b = c1.y - a * c1.x;

    //assert(c4.x != c3.x); // TODO temporarely removed
    
    float c = (c4.y - c3.y) / (c4.x - c3.x);
    float d = c3.y - c * c3.x;

    float x, y;
    
    //assert(a != c); // TODO temporarely removed
    if(a==c)
        return true;
    x = (d - b) / (a - c);
    y = (d * a - b * c) / (a - c);

    if(fabsf(x - c1.x) < MIN_DIFF && fabsf(y - c1.y) < MIN_DIFF)
        return false;
    if(fabsf(x - c2.x) < MIN_DIFF && fabsf(y - c2.y) < MIN_DIFF)
        return false;
    if(fabsf(x - c3.x) < MIN_DIFF && fabsf(y - c3.y) < MIN_DIFF)
        return false;
    if(fabsf(x - c4.x) < MIN_DIFF && fabsf(y - c4.y) < MIN_DIFF)
        return false;

    if( 
        x > (fmin(c1.x, c2.x)) && x < (fmax(c1.x, c2.x)) && x > (fmin(c3.x, c4.x)) && x < (fmax(c3.x, c4.x)) &&
        y > (fmin(c1.y, c2.y)) && y < (fmax(c1.y, c2.y)) && y > (fmin(c3.y, c4.y)) && y < (fmax(c3.y, c4.y)))
        return true;
    return false;
}

bool inside_cycle(int *vertices, int vert_num, int io_ind) // TODO instead of lines_cross use min or max distance
{
    assert(vert_num >= 3);
    if(vert_num<3)
        return false;
    
    int i,j;
    float v_dis =0.0;
    float io_dis=0.0;
    for(i=0; i<vert_num; i++)   
    {
        for(j=0; j<vert_num; j++)
        {
            if(i==j)
                continue;
            city ci,cj;
            city_info(&ci,vertices[i],READ);
            city_info(&cj,vertices[j],READ);
            if(distance(ci, cj) > v_dis)
                v_dis=distance(ci, cj); 
        }
    }
    
    city io_city, v_city;
    city_info(&io_city, io_ind, READ);
    for(i=0; i<vert_num; i++)
    {
        city_info(&v_city, vertices[i], READ);
        if(distance(v_city, io_city) > io_dis)
            io_dis=distance(v_city, io_city);
    }
    if(v_dis > io_dis)
        return true;
    else
    {
        assert(io_dis > v_dis);
        return false;
    }
    
    
    /*
    city in_out;
    city_info(&in_out, io_ind, READ);// TODO compute and find these
    
    
    int v3[3];
    bool er = vertex3(v3);
    if(er == false)
        return false;
    printf("v1 is %d v2 is %d v3 is %d io_ind is %d\n", v3[0], v3[1], v3[2],io_ind);
    

    
    
        bool cond = false;
        int j = 0;
        int k = 0;
        int l = 0;
        for(l = 0; l < vert_num; l++)
        {
            if(l==vert_num-1)
                j=0;
            else 
                j=l+1;
            
                if(l == j)
                    continue;

                city cl, cj;
                city_info(&cl, vertices[l], READ);
                city_info(&cj, vertices[j], READ);
                for(k = 0; k < vert_num; k++)// CITY_NUM
                {
                    
                    if(vertices[l]==vertices[j] || vertices[k]==vertices[j] || vertices[k]==vertices[l])
                        continue;
                    if(vertices[k] == io_ind)
                        continue;
                    
                    
                    city ck;
                    city_info(&ck, vertices[k] , READ);
                    if(lines_cross(ck, in_out, cl, cj)
                     
                    ) 
                    
                    { 
                        printf("** ");
                        print_vertices(vertices,vert_num);
                        
                        cond = true;
                        return false; // TODO is it correct ?
                    }
                    
                }
            //}
        }
        
        if(cond)
        {
            printf("io_ind is %d ,out\n", io_ind);
            return false;
            
        }
        else
        {
            printf("io_ind is %d ,in\n", io_ind);
            return true;
            
        }
    
    
    return true;*/
}
bool outside_cycle(int *vertices, int vert_num, int io_ind)
{
    bool r;
    
    int i;
    for(i = 0; i < vert_num; i++)
    {
        city ci;
        city_info(&ci, i, READ);

        if(io_ind == vertices[i])
            return false;
    }
    r = !inside_cycle(vertices, vert_num, io_ind);
    return r;
}

void print_nexts(void)
{
    city c0;
    int prev_i=0;
    city_info(&c0,0,READ);
    int next_i=c0.next_i;
    for(int n=0; n< CITY_NUM; n++)
    {
        printf("prev_i is %d next_i is %d\n",prev_i,next_i);
    
        city from,to;
        city_info(&from, prev_i, READ);
        prev_i=from.next_i;
        city_info(&to, next_i, READ);//j+1
        //best_path[i] = next; 
        //next = ci.next_i;
        //cj.next_i=best_path[j+1];
        //city_info(&cj, j, WRITE);
        prev_i=next_i;
        next_i=to.next_i;
        
    }
}

void print_vertices(int const *vertices, int vert_num)
{
    int i;
    assert(vert_num>=0);
    assert(vert_num<=CITY_NUM);
    for(i = 0; i < vert_num; i++)
        printf("$ %d ", vertices[i]);// vertices[%d] is
    printf("\n");
}

void print_no_cycle(void)
{
    int i;
    
    printf("cycles : ");
    for(i = 0; i < CITY_NUM; i++)
    {
        city ci;
        city_info(&ci, i, READ);
        
        printf(" cycle[%d] is %d ", i, ci.on_cycle);
    }
    printf("\n");
}

void expand_cycle(int *vertices, int vert_num, int out_ind /*, int cycle*/)
{
    assert(vert_num>=3);

    int i, j;
    int v1 = NO_CITY, v2 = NO_CITY;
    float dis1 = X_MAX + Y_MAX;
    float dis2 = X_MAX + Y_MAX;
    city out_city;
    city_info(&out_city, out_ind, READ);
    assert(!inside_cycle(vertices, vert_num, out_ind));
    for(i = 0; i < vert_num; i++)
        assert(out_ind != vertices[i]);

    print_vertices(vertices, vert_num);
    for(i = 0; i < vert_num - 1; i++)
        for(j = i + 1; j < vert_num; j++)
        {
            if(i == j)
                continue;

            city ci;
            city_info(&ci, vertices[i], READ);
            city cj;
            city_info(&cj, vertices[j], READ);

            if(distance(ci, out_city) < dis1 && //!lines_cross(ci, out_city, ci, cj) &&
               vertices[i] != v2) // TODO no cross
            {
                dis1 = distance(ci, out_city);
                v1 = vertices[i];
            }

            if(distance(cj, out_city) < dis2 //&& !lines_cross(cj, out_city, ci, cj) 
                && vertices[j] != v1) 
            {
                dis2 = distance(cj, out_city);
                
                v2 = vertices[j];
            }
        }
    if(v1 == v2)
        printf("v1 is %d v2 is %d out_ind is %d \n", v1, v2, out_ind);
    assert(v1 != v2);

    if(v2 == vertices[vert_num - 1] || v1 == vertices[vert_num - 1])
    {
        /*city cout;
        city_info(&cout, out_ind, READ);
        cout.on_cycle=cycle;
        city_info(&cout, out_ind, WRITE);*/
        
        vertices[vert_num] = out_ind;
        print_vertices(vertices, vert_num + 1);
        return;
    }
    for(i = vert_num - 1; i > 0; i--)
    {
        vertices[i + 1] = vertices[i];
        if(vertices[i] == v2 || vertices[i] == v1) 
        {
            vertices[i] = out_ind;
            break;
        }
    }

    /*city cout;
    city_info(&cout, out_ind, READ);
    cout.on_cycle=cycle;
    city_info(&cout, out_ind, WRITE);*/
        
    print_vertices(vertices, vert_num + 1);
    //exit(1);
}

// TODO expand cycle 3 to more.
bool general_cycle(int *vertices, int *vert_num, int cycle) 
{
    //int *vertices=*v;
    // TODO inside vertices
    *vert_num = 0;
    int i;
    int cnt; 

    cnt = 3;
    bool er = vertex3(vertices);
    
    if(!er)
    {
        city ci;
        for(i = 0; i < CITY_NUM; i++)
        {
            city_info(&ci, i, READ);
            if(ci.on_cycle == NOT_OC)
            {
                ci.on_cycle = cycle;
                city_info(&ci, i, WRITE);
                cycle_keeper(&vertices[i] , i, cycle, WRITE); // &i &vertices[i]
                vertices[*vert_num] = i;
                (*vert_num)++;
            }
        }
        print_vertices(vertices, *vert_num);
        return true; // LAST_LAYER;
    }

    for(i = 0; i < CITY_NUM; i++) 
    {
        
        city in_out;
        city_info(&in_out, i, READ);
        bool op;
        op = outside_cycle(vertices, cnt, i); 
        if(op && in_out.on_cycle==NOT_OC) // added second condition
        {
            assert(!inside_cycle(vertices,cnt,i));
            expand_cycle(vertices, cnt, i);
            cnt++;
            
            print_vertices(vertices, cnt);
        }
    }
    print_vertices(vertices, cnt);
    for(i = 0; i < cnt; i++)
    {
        city ci;
        city_info(&ci, vertices[i], READ);
        assert(ci.on_cycle <= cycle);
        
        ci.on_cycle = cycle;
        city_info(&ci, vertices[i], WRITE);
        cycle_keeper(&vertices[i], i, cycle, WRITE); 
    }

    *vert_num = cnt;
    return false; 
}

//#define EPS 0.01
#define IMP_COOR_VALUE -1.0
struct xy // for sorting
{
    float x;
    float y;
};
bool imp_coor(float coor)
{
    if(fabs(coor-IMP_COOR_VALUE)<EPS)
        return true;
    return false;
}
bool vertices_ok(int nvert, float *vertx, float *verty)
{
    int i;
    for(i=0; i<nvert; i++)
    {
        if(imp_coor(vertx[i]) || fabs(vertx[i])<EPS)
            return false;
        if(imp_coor(verty[i]) || fabs(verty[i])<EPS)
            return false;
    }
    return true;
}
int is_left(int nvert, float *vertx, float *verty, int a, int b, int c)
{
    assert(a>=0 && a<nvert);
    assert(b>=0 && b<nvert);
    assert(c>=0 && c<nvert);
    int r;
    r = (vertx[b]-vertx[a])*(verty[c]-verty[a])-(verty[b]-verty[a])*(vertx[c]-vertx[a]) >0 ;
    return r;
}
int cmp(const void* elem1, const void* elem2)
{
    struct xy xy1 = *((struct xy *)elem1);
    struct xy xy2 = *((struct xy *)elem2);
    if(xy1.x > xy2.x)
        return 1;
    else
        return -1;
}
int rev_cmp(const void* elem1, const void* elem2)
{
    return -cmp(elem1, elem2);
}
void create_polygon(int nvert, float **vertx, float **verty) 
{
    assert(nvert>=3);
    assert(vertices_ok(nvert, *vertx, *verty));
    
    int p=0; // left most
    float px,py;
    int q=0; // right most
    float qx, qy;
    int i;
    for(i=0; i<nvert; i++)
    {
        if( (*verty)[i]<(*verty)[p] )
            p=i;
        if((*verty)[i]>(*verty)[q])
            q=i;
    }
    assert(p!=q);
    px=(*vertx)[p];
    py=(*verty)[p];
    qx=(*vertx)[q];
    qy=(*verty)[q];
    struct xy * below=calloc(nvert+1, sizeof(struct xy));
    struct xy * above=calloc(nvert+1, sizeof(struct xy));
    int bi=0;
    int ai=0;
    
    struct xy all[nvert+1];
    for(i=0; i<nvert; i++)
    {
        all[i].x=(*vertx)[i];
        all[i].y=(*verty)[i];
    }
    for(i=0; i<nvert; i++)
        if(is_left(nvert, *vertx, *verty, p, q, i))
        {
            if(i==p || i==q)
                continue;
            above[ai].x=(*vertx)[i];
            above[ai].y=(*verty)[i];
            ai++;
        }
        else
        {
            if(i==p || i==q)
                continue;
            below[bi].x=(*vertx)[i];
            below[bi].y=(*verty)[i];
            bi++;
        }
    assert(nvert==(ai+bi+2));
    
    qsort(above, ai, sizeof(struct xy), rev_cmp );
    qsort(below, bi, sizeof(struct xy), cmp );
    
    (*vertx)[0]=px;
    (*verty)[0]=py;
    for(i=1; i<=bi; i++)
    {
        (*vertx)[i]=below[i-1].x;
        (*verty)[i]=below[i-1].y;
    }
    (*vertx)[i]=qx;
    (*verty)[i]=qy;
    i++;
    for(;i<nvert; i++)
    {
        (*vertx)[i]=above[i-bi-2].x;
        (*verty)[i]=above[i-bi-2].y;
    }
    
    for(i=0; i<nvert; i++) // some asserts
    {
        assert(!imp_coor((*vertx)[i]));
        assert(!imp_coor((*verty)[i]));
    }
    int j;
    city cv[4];
    for(i=0; i<nvert-2; i++)
        for(j=i+1; j<nvert-1; j++)
        {
            cv[0].x=(*vertx)[i];
            cv[0].y=(*verty)[i];
            cv[1].x=(*vertx)[i+1];
            cv[1].y=(*verty)[i+1];
            cv[2].x=(*vertx)[j];
            cv[2].y=(*verty)[j];
            cv[3].x=(*vertx)[j+1];
            cv[3].y=(*verty)[j+1];
            printf("nvert is %d i is %d j is %d\n",nvert,i,j);
            assert(!lines_cross(cv[0],cv[1],cv[2],cv[3]));
        }
    
    free(below);
    free(above);
    assert(vertices_ok(nvert, *vertx, *verty));
}
bool p_is_a_vertex(int nvert, float *vertx, float *verty, float x, float y)
{
    assert(nvert<=CITY_NUM);
    assert(!imp_coor(x));
    assert(!imp_coor(y));
    int i;
    for(i=0; i<nvert; i++)
    {
        assert(!imp_coor(vertx[i]));
        assert(!imp_coor(verty[i]));
        if( (fabs(vertx[i]-x)<EPS) && (fabs(verty[i]-y)<EPS) )
            return true;
    }
    return false;
}
int pnpoly(int nvert, float *vertx, float *verty, float testx, float testy) // taken from internet
{ // TODO single array of cities as argument above
    assert(nvert>=3);
    assert(nvert<=CITY_NUM);
    assert(!imp_coor(testx));
    assert(!imp_coor(testy));
    if(p_is_a_vertex(nvert, vertx, verty, testx, testy))
        return 0;
    int i, j, c = 0;
    for (i = 0, j = nvert-1; i < nvert; j = i++) 
    {
        if ( ((verty[i]>testy) != (verty[j]>testy)) &&
	       (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
            c = !c;
    }
    return c;
}
void remove_verts(int * nvert, float ** vertx, float ** verty, int ni)
{
    (*nvert)--;
    (*vertx)[ni]=(*vertx)[*nvert];
    (*verty)[ni]=(*verty)[*nvert];
    (*vertx)[*nvert]=IMP_COOR_VALUE;
    (*verty)[*nvert]=IMP_COOR_VALUE;
    assert(vertices_ok(*nvert, *vertx, *verty));
    create_polygon(*nvert,vertx, verty);
}
void init_verts(int * nvert, float ** vertx, float ** verty)
{
    int i;
    for(i=0; i<CITY_NUM; i++)
    {
        (*vertx)[i]=IMP_COOR_VALUE;
        (*verty)[i]=IMP_COOR_VALUE;
    }
    (*nvert)=0;
}
bool general_cycle_version2(int * vertices,int* vert_num,int cycle) // TODO add cycle_keeper
{// TODO debug it
    assert(CITY_NUM>=3);
    assert(cycle != NOT_OC);
    int nvert = 0;
    int i,j;
    (*vert_num)=0;
    
    float * vertx = malloc( (CITY_NUM+1)*sizeof(float) );
    float * verty = malloc( (CITY_NUM+1)*sizeof(float) );
    init_verts(&nvert, &vertx, &verty);
    
    if(cycle == 1) // first cycle , yet no polygon
    {
        
        city ci;
        for(i=0 ; i<=2; i++)
        {
            city_info(&ci, i, READ);
            assert(ci.on_cycle == NOT_OC); // TODO second condition
            ci.on_cycle=cycle;
            city_info(&ci, i, WRITE);
            cycle_keeper(&vertices[i] , i, cycle, WRITE); 
            vertices[*vert_num]=i;
            (*vert_num)++;
            vertx[i]=ci.x;
            verty[i]=ci.y;
        }
        // TODO debug following:
        nvert=3; 
        for(j=CITY_NUM-1; j>=0; j--)
        {
            city cj;
            city_info(&cj, j, READ);
            
            /*for(int ni=0; ni<nvert; ni++)
                if(fabs(cj.x-vertx[ni])<EPS && fabs(cj.y-verty[ni])<EPS)
                {
                    remove_verts(&nvert, &vertx, &verty, ni);
                    create_polygon(nvert, &vertx, &verty);
                    break;
                }*/
            if(p_is_a_vertex(nvert, vertx, verty, cj.x, cj.y))
            {
                remove_verts(&nvert, &vertx, &verty, j);
                create_polygon(nvert, &vertx, &verty);
            }
            if(pnpoly(nvert, vertx, verty, cj.x, cj.y))
            {
                cj.on_cycle=NOT_OC;
                city_info(&cj, j, WRITE);
                
            }
            else 
            {
                cj.on_cycle=cycle;
                city_info(&cj, j, WRITE);
                
                vertx[nvert]=cj.x;
                verty[nvert]=cj.y;
                nvert++;
                create_polygon(nvert, &vertx, &verty);
                
            }
        }
    }
    else
    {
        assert(cycle>=2);
        city ci;
        for(i=0; i<CITY_NUM; i++)
        {
            city_info(&ci, i, READ);
            if(ci.on_cycle==NOT_OC)
            {
                ci.on_cycle=cycle;
                city_info(&ci, i, WRITE);
                vertices[*vert_num]=i;
                (*vert_num)++;
            }
        }
    }
        
    // there is a polygon
    init_verts(&nvert, &vertx, &verty);
    
    for(i = 0; i < CITY_NUM; i++)
    {
        city ci;
        city_info(&ci, i, READ);
        assert(!imp_coor(ci.x));
        assert(!imp_coor(ci.y));
        if( ci.on_cycle == cycle)
        {
            vertx[nvert] = ci.x;
            verty[nvert] = ci.y;
            nvert++;
            assert(vertices_ok(nvert, vertx, verty));
            //(*vert_num)++;
        }
        
    }
    
    (*vert_num)=nvert;
    int nvi=0;
    if(nvert<3)
    {
        for(i = 0; i < CITY_NUM; i++)
        {
            city ci;
            city_info(&ci, i, READ);
            if(ci.on_cycle==NOT_OC)
            {
                ci.on_cycle = cycle;
                city_info(&ci,i,WRITE);
                cycle_keeper(&vertices[i] , i, cycle, WRITE); 
                vertices[nvi]=i;
                nvi++;
            }
        }
        (*vert_num)=nvert;
        return false;
    }
    assert(nvert>=3);    
    
    for(i = 0; i < CITY_NUM; i++) // expand or reduce
    {
        city ci;
        city_info(&ci, i, READ);
        assert(vertices_ok(nvert, vertx, verty));
        create_polygon(nvert,&vertx, &verty); // TODO add more below
        if( !pnpoly(nvert,vertx,verty,ci.x,ci.y)
            &&!p_is_a_vertex(nvert,vertx,verty,ci.x,ci.y) 
            && (ci.on_cycle == NOT_OC)
            )
        {
            ci.on_cycle = cycle;
            city_info(&ci,i,WRITE);
            cycle_keeper(&vertices[i] , i, cycle, WRITE);
            vertices[*vert_num]=i;
            (*vert_num)++;
            
            vertx[nvert]=ci.x;
            verty[nvert]=ci.y;
            nvert++;
            assert(vertices_ok(nvert, vertx, verty));
            create_polygon(nvert,&vertx, &verty);
            
            assert((*vert_num)==nvert);
            /*int n=nvert;
            for(int j=0; j<n; j++) // newly added
            {
                city cv;
                city_info(&cv, vertices[j], READ); 
                if(pnpoly(n, vertx, verty, cv.x, cv.y)&& !p_is_a_vertex(n,vertx,verty,cv.x,cv.y))
                {
                    cv.on_cycle=NOT_OC;
                    city_info(&cv, vertices[j], WRITE);
                    (*vert_num)--;
                    vertices[j]=vertices[*vert_num];
                    vertices[*vert_num]=0;
                    
                }
            }
            for(int j=0; j<n; j++)
            {
                
            }*/
            
            for(j=0; j<CITY_NUM ; j++) // just disabled && j!=i
            {
                city cj;
                city_info(&cj, j, READ);
                // TODO remove cj coor from vertx,verty
                if(//cj.on_cycle==cycle &&
                    pnpoly(nvert,vertx,verty,cj.x,cj.y) 
                    //&&!p_is_a_vertex(nvert,vertx,verty,cj.x,cj.y) 
                    )// cj is in polygon now 
                {
                    cj.on_cycle=NOT_OC;
                    city_info(&cj,j,WRITE);
                    cycle_keeper(&vertices[j] , j, cycle, WRITE); 
                    for(int k=0; k<*vert_num; k++)
                        if(vertices[k]==j)
                        {
                            (*vert_num)--;
                            vertices[k]=vertices[*vert_num];
                            vertices[*vert_num]=0;
                            //break;
                        }
                    
                    for(nvi=0; nvi<nvert; nvi++)
                        if(fabs(vertx[nvi]-cj.x)<EPS && fabs(verty[nvi]-cj.y)<EPS)
                        {
                            remove_verts(&nvert, &vertx, &verty, nvi);
                            //break;
                        }
                }
            }
        }
        
    }
    
    /*for(i=0; i<CITY_NUM; i++) // TODO fix for wrong on_cycle, maybe extra
    {
        city ci;
        city_info(&ci, i, READ);
        if(ci.on_cycle==cycle )
        {
            assert(p_is_a_vertex(nvert,vertx,verty,ci.x, ci.y));
            for( j=0; j<CITY_NUM && j!=i; j++)
            {
                city cj;
                city_info(&cj, j, READ);
                if(pnpoly(nvert,vertx,verty,cj.x,cj.y) && !p_is_a_vertex(nvert,vertx,verty,cj.x,cj.y) )//&& cj.on_cycle!=(cycle+1)
                {
                    cj.on_cycle=NOT_OC;
                    //assert(cj.on_cycle==NOT_OC);
                    city_info(&cj, j, WRITE);
                }
            }
        }
    }*/
    
    (*vert_num)=nvert;
    free(vertx);
    free(verty);
    return true;
}

// ----------------------------------------------------//
bool ccw(const city *a, const city *b, const city *c) {
    return (b->x - a->x) * (c->y - a->y)
         > (b->y - a->y) * (c->x - a->x);
}
int compare_cities(const void *lhs, const void *rhs) {
    const city* lp = lhs;
    const city* rp = rhs;
    if (lp->x < rp->x)
        return -1;
    if (rp->x < lp->x)
        return 1;
    if (lp->y < rp->y)
        return -1;
    if (rp->y < lp->y)
        return 1;
    return 0;
}
void fatal(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}
void* xmalloc(size_t n) {
    void* ptr = malloc(n);
    if (ptr == NULL)
        fatal("Out of memory");
    return ptr;
}
void* xrealloc(void* p, size_t n) {
    void* ptr = realloc(p, n);
    if (ptr == NULL)
        fatal("Out of memory");
    return ptr;
}
bool same_city(city c1, city c2)
{
    if(strcmp(c1.name, c2.name)==0)
        {
            assert( fabs(c1.x-c2.x)<EPS);
            assert(fabs(c1.y-c2.y)<EPS);
            assert(c1.visited==c2.visited);
            assert(c1.on_cycle==c2.on_cycle);
            assert(c1.joint==c2.joint);
            assert(c1.next_i==c2.next_i);
            return true;
        }
    return false;
}

bool convex_hull(int * vertices, int * vert_num,int cycle)
{
    int i;
    city ci;
    bool flag=false;
    city * curr_cities=xmalloc( (CITY_NUM+1) * sizeof(city));
    int cci=0;
    
    for(i=0; i<CITY_NUM; i++)
    {
        city_info(&ci, i, READ);
        if(ci.on_cycle==NOT_OC)
        {
            curr_cities[cci]=ci;
            cci++;
            flag=true;
        }
    }
    if(flag==false) // all cities assigned.
    {
        (*vert_num) = 0;
        vertices = NULL;
        return false;
    }
    
    int size = 0;
    int capacity = 4;
    city* hull = xmalloc(capacity * sizeof(city));
    
    qsort(curr_cities, cci, sizeof(city), compare_cities);
    
    /* lower hull */
    for (i = 0; i < cci; ++i) {
        while (size >= 2 && !ccw(&hull[size - 2], &hull[size - 1], &curr_cities[i]))
            --size;
        if (size == capacity) {
            capacity *= 2;
            hull = xrealloc(hull, capacity * sizeof(city));
        }
        assert(size >= 0 && size < capacity);
        hull[size++] = curr_cities[i];
    }
    
    /* upper hull */
    int t = size + 1;
    for (i = cci - 1; i >= 0; i--) {
        while (size >= t && !ccw(&hull[size - 2], &hull[size - 1], &curr_cities[i]))
            --size;
        if (size == capacity) {
            capacity *= 2;
            hull = xrealloc(hull, capacity * sizeof(city));
        }
        assert(size >= 0 && size < capacity);
        hull[size++] = curr_cities[i];
    }
    --size;
    assert(size >= 0);
    hull = xrealloc(hull, size * sizeof(city));
    
    city cj;
    int j;
    (*vert_num) = 0;
    for(i=0; i<size; i++) // TODO maybe extra , modify above
    {
        ci = hull[i];
        for(j=0; j<CITY_NUM; j++)
        {
            city_info(&cj, j, READ);
            if(same_city(cj,ci))
            {
                cj.on_cycle=cycle;
                city_info(&cj, j, WRITE);
                vertices[*vert_num]=j;
                (*vert_num)++;
            }
        }
    }
    // vertices
    (*vert_num) = size;
    
    free(curr_cities);
    free(hull);
    return true;
}
///-------------------------------------------------------------------------------///
void print_joints(int cycle)
{
    printf("joints(cycle is %d): ", cycle);
    int i;
    for(i = 0; i < CITY_NUM; i++)
    {
        city ci;
        city_info(&ci, i, READ);
        if(ci.on_cycle == cycle && ci.joint!=NO_CITY)
            printf("vertex[%d] is joint to %d ", i, ci.joint);
    }

    printf("\n");
}

// TODO joints between neighbor cycles
// joint outside , examine both directions of joints
void joints(int cycle)  // TODO render and correct joints
{
    assert(cycle >= NOT_OC);
    assert(cycle <= MAX_CYCLES);
    int i;
    int j_cnt = 0;
    int oj_cnt = 0;
    city ci;
    int *in_joint = calloc(CITY_NUM+1, sizeof(int)); 
    int *out_joint = calloc(CITY_NUM+1, sizeof(int)); 

    for(i = 0; i < CITY_NUM; i++)
    {
        city_info(&ci, i, READ);
        if(ci.on_cycle == cycle )//&& ci.joint==NO_CITY)
        {
            in_joint[j_cnt] = i;
            j_cnt++;
        }
    }
    for(i = 0; i < CITY_NUM; i++)
    {
        city_info(&ci, i, READ);
        if(ci.on_cycle == (cycle - 1) )//&& ci.joint==NO_CITY)
        {
            out_joint[oj_cnt] = i;
            oj_cnt++;
        }
    }
    
    printf("cycle is %d oj_cnt is %d j_cnt is %d\n",cycle,oj_cnt,j_cnt);
    //assert(oj_cnt >= 3); // TODO for now removed
    
    if(oj_cnt<=2)
    {
        free(in_joint);
        free(out_joint);
        return;
    }

    if(j_cnt == 0)
        return;
    if(j_cnt == 1) // TODO debug
    {
        city cji, cjo;
        city_info(&cji, in_joint[0], READ);

        city_info(&cjo, out_joint[0], READ);
        float dis1 = distance(cji, cjo);
        cjo.joint = in_joint[0];
        city_info(&cjo, out_joint[0], WRITE);

        for(i = 1; i < oj_cnt; i++)
        {
            city_info(&cjo, out_joint[i], READ);
            if(distance(cji, cjo) < dis1)
            {
                dis1 = distance(cji, cjo);
                int j;
                city prev_city;
                for(j = 0; j < i; j++)
                {
                    city_info(&prev_city, out_joint[j], READ);
                    prev_city.joint = NO_CITY;
                    city_info(&prev_city, out_joint[j], WRITE);
                }
                cjo.joint = in_joint[0];
                city_info(&cjo, out_joint[i], WRITE);
            }
        }
        
        city_info(&cji, in_joint[0], READ);

        city_info(&cjo, out_joint[0], READ);
        float dis2 = distance(cji, cjo);
        cjo.joint = in_joint[0];
        city_info(&cjo, out_joint[0], WRITE);
        for(i = 0; i < oj_cnt; i++)
        {
            city_info(&cjo, out_joint[i], READ);
            if(cjo.joint != NO_CITY)
                continue;
            if(distance(cji, cjo) < dis2)
            {
                dis2 = distance(cji, cjo);
                
                cjo.joint = in_joint[0];
                city_info(&cjo, out_joint[i], WRITE);
            }
        }
    }
    if(j_cnt >= 2)
    {
        int j;
        city cji, cjo;
        city_info(&cji, in_joint[0], READ);
        city_info(&cjo, out_joint[0], READ);
        float dis1 = distance(cji, cjo); // TODO dis=MAX_DIST
        int chosen_ci = in_joint[0];

        cjo.joint = in_joint[0];
        city_info(&cjo, out_joint[0], WRITE);
        for(i = 0; i < oj_cnt; i++)
            for(j = 0; j < j_cnt; j++)
            {
                city_info(&cji, in_joint[j], READ);
                city_info(&cjo, out_joint[i], READ);
                if(cjo.joint != NO_CITY)
                    continue;
                if(distance(cji, cjo) < dis1)
                {
                    dis1 = distance(cji, cjo);
                    int k;
                    city prev_city;
                    for(k = 0; k < i; k++)
                    {
                        city_info(&prev_city, out_joint[k], READ);
                        prev_city.joint = NO_CITY;
                        city_info(&prev_city, out_joint[k], WRITE);
                    }
                    cjo.joint = in_joint[j];
                    city_info(&cjo, out_joint[i], WRITE);
                    chosen_ci = in_joint[j];
                }
            }
        
        city_info(&cji, chosen_ci, READ);
        city_info(&cjo, out_joint[0], READ);
        if(cjo.joint != NO_CITY)
        {
            city_info(&cjo, out_joint[1], READ);
            cjo.joint = chosen_ci;
            city_info(&cjo, out_joint[1], WRITE);
        }
        else
        {
            cjo.joint = chosen_ci;
            city_info(&cjo, out_joint[0], WRITE);
        }
        dis1 = distance(cjo, cji);

        for(i = 0; i < oj_cnt; i++)
        {
            city_info(&cjo, out_joint[i], READ);
            if(cjo.joint != NO_CITY)
                continue;
            if(distance(cji, cjo) < dis1)
            {
                dis1 = distance(cji, cjo);
                
                cjo.joint = chosen_ci;
                city_info(&cjo, out_joint[i], WRITE);
            }
        }

        // TODO 2nd joint
        city_info(&cjo, out_joint[0], READ);
        if(cjo.joint != NO_CITY)
            city_info(&cjo, out_joint[1], READ);
        city_info(&cji, in_joint[0], READ);
        if(cji.joint != NO_CITY)
            city_info(&cji, in_joint[1], READ);

        float dis2 = distance(cji, cjo);
        for(i = 0; i < oj_cnt; i++)
            for(j = 0; j < j_cnt; j++)
            {

                city_info(&cji, in_joint[j], READ);
                city_info(&cjo, out_joint[i], READ);
                if(cjo.joint != NO_CITY || cji.joint != NO_CITY)
                    continue;

                if(distance(cji, cjo) < dis2)
                {
                    dis2 = distance(cji, cjo);
                    int k;
                    city prev_city;
                    for(k = 0; k < i; k++)
                    {
                        city_info(&prev_city, out_joint[k], READ);
                        prev_city.joint = NO_CITY;
                    }

                    cjo.joint = in_joint[j];
                    city_info(&cjo, out_joint[i], WRITE);
                    chosen_ci = in_joint[j];
                    printf("oji is %d ijj is %d\n", out_joint[i], in_joint[j]);
                }
            }
        
        city_info(&cji, chosen_ci, READ);
        city_info(&cjo, out_joint[0], READ);
        for(i = 0; cjo.joint != NO_CITY && i < oj_cnt; i++)
        {
            city_info(&cjo, out_joint[i], READ);
        }
        if(i >= oj_cnt)
            return;
        cjo.joint = chosen_ci;
        
        dis2 = distance(cjo, cji);
        for(i = 0; i < oj_cnt; i++)
        {
            city_info(&cjo, out_joint[i], READ);
            if(cjo.joint != NO_CITY)
                continue;
            if(distance(cji, cjo) < dis2)
            {
                dis2 = distance(cji, cjo);
                
                int k;
                city prev_city;
                for(k = 0; k < i; k++)
                {
                    city_info(&prev_city, out_joint[k], READ);
                    prev_city.joint = NO_CITY;
                }
                cjo.joint = chosen_ci;
                city_info(&cjo, out_joint[i], WRITE);
            }
        }
        // TODO joints for all (NO_CITY) oj to nearest ij
        int nc_cnt=0; 
        for(i=0; i<oj_cnt; i++)
        {
            city_info(&cjo, out_joint[i], READ);
            city_info(&cji, in_joint[0], READ);
            float dis3 = distance(cjo,cji);
            if(cjo.joint == NO_CITY)
            {
                for(j=0; j<j_cnt; j++)
                {
                    city_info(&cji, in_joint[j], READ);
                    if(distance(cjo,cji) < dis3)
                    {
                        dis3 = distance(cjo,cji);
                        cjo.joint = j;
                        city_info(&cjo, out_joint[i], WRITE);
                        nc_cnt++;
                    }
                }
            }
        }
        /*for(i=0; i<oj_cnt; i++)
        {
            city_info(&cjo, out_joint[i], READ);
            if(cjo.joint == NO_CITY)
                exit(1); // TODO remove this 
        }*/
        printf("nc_cnt is %d\n",nc_cnt);
        
    }

    // TODO print joints
    print_joints(cycle - 1);
    print_joints(cycle);

    free(in_joint);
    free(out_joint);
    return;
} 

// ------------------------------------------------------------- //
bool city_on_vertices(int i, int *const vertices, int vcnt)
{
    bool cov = false;
    int j;
    for(j = 0; j < vcnt; j++)
        if(i == vertices[j])
            cov = true;
    return cov;
}

#define MAX_NEXT 10 // max 7
int *next_city(int const *const vertices, int vcnt, int i) // TODO debug
{
    print_vertices(vertices,vcnt);
    for(int ind=0; ind<vcnt; ind++)
    {
        city vc;
        city_info(&vc, vertices[ind], READ);
        printf(" %d ",vc.on_cycle);
    }
    printf("\n");

    int *next_node = calloc(MAX_NEXT, sizeof(int));
    for(int ni_ctr = 0; ni_ctr < MAX_NEXT; ni_ctr++)
        next_node[ni_ctr] = NO_CITY;

    city ci, cj;
    city_info(&ci, i, READ);
    int out_cycle = ci.on_cycle - 1;
    int j;

    for(j = 0; j < CITY_NUM; j++) // TODO two values
    {
        city_info(&cj, j, READ);
        if(cj.on_cycle == out_cycle && cj.joint == i)
        {
            next_node[0] = j;
            break;
        }
    }
    for(; j < CITY_NUM; j++)
    {
        city_info(&cj, j, READ);
        if(cj.on_cycle == out_cycle && cj.joint == i)
            next_node[1] = j;
    }

    for(j = 0; (j < vcnt) ; j++)//CITY_NUM
    {
        city_info(&cj, vertices[j], READ); // vertices[j]
        if(ci.on_cycle != cj.on_cycle)
            continue;
        if(i==vertices[j]) // vertices[j]
            continue;
        

        int i1 = (((j + 1) >= vcnt) ? 0 : (j + 1));
        int i2 = (((j - 1) < 0) ? vcnt - 1 : (j - 1));
        
        city c1,c2;
        city_info(&c1, vertices[i1], READ);
        city_info(&c2, vertices[i2], READ);
        assert(c1.on_cycle==c2.on_cycle);
        
        if(i == vertices[j]) 
        {
            if(next_node[2] == NO_CITY) // TODO gen this pattern
                next_node[2] = vertices[i1]; 
        }
        else if( i == vertices[i1]) 
        {
            if(next_node[3] == NO_CITY)
                next_node[3] = vertices[j];
        }
        if( i == vertices[j]) // TODO rotation ?
        {
            if(next_node[4] == NO_CITY)
                next_node[4] = vertices[i2];
        }
        else if( i == vertices[i2])
        {
            if(next_node[5] == NO_CITY)
                next_node[5] = vertices[j];
        }
        
    }
    for(j = 0; j < CITY_NUM; j++) // TODO two values
    {
        city_info(&cj, j, READ);
        if(ci.joint == j)
            next_node[6] = j;
    }

    return next_node;
}

void nexts_keeper(int *nexts, int city_num, enum RW rw)
{
    assert(city_num >= 0);
    assert(city_num < CITY_NUM);
    static int flag = 0;
    static int *nexts_arr;

    if(flag == 0)
    {
        nexts_arr = calloc((CITY_NUM) * (MAX_NEXT), sizeof(int)); 
        flag = 1;
    }

    int index = city_num * MAX_NEXT;
    assert(index >= 0);
    assert(index < CITY_NUM * MAX_NEXT);

    if(rw == READ)
    {
        for(int j = 0; j < MAX_NEXT; j++)
            nexts[j] = nexts_arr[index + j]; 
    }
    else // WRITE
    {
        for(int j = 0; j < MAX_NEXT; j++)
            nexts_arr[index + j] = *(nexts + j);
    }
    // TODO FREE
}

int compare_ints(const void *a, const void *b) 
{
    int arg1 = *(const int *)a;
    int arg2 = *(const int *)b;

    if(arg1 < arg2)
        return 1;
    if(arg1 > arg2)
        return -1;
    return 0;
}

void print_keeper(void)
{
    int i;
    printf("------\n");
    int nk[MAX_NEXT] = {-1};
    for(i = 0; i < CITY_NUM; i++)
    {
        printf("%d : ", i);
        nexts_keeper(nk, i, READ);

        for(int j = 0; j < MAX_NEXT; j++)
        {
            if(nk[j]!=NO_CITY)
                printf("%d,", nk[j]);
            else
                printf("-,");
        }
        printf("\n");
    }
    return;
}
void remove_repetition(void)
{
    int i;
    int nk[MAX_NEXT] = {-1};
    for(i = 0; i < CITY_NUM; i++)
    {
        nexts_keeper(nk, i, READ);
        for(int j = 0; j < MAX_NEXT - 1; j++)
            if(nk[j] == nk[j + 1] && nk[j] != NO_CITY)
            {
                for(int k = j; k < MAX_NEXT - 1; k++)
                    nk[k] = nk[k + 1];
            }

        nexts_keeper(nk, i, WRITE);
    }
    return;
}

/// ________________________________________________________ ///

int g_vertex[CITY_NUM * MAX_NEXT]; // TODO remove global array
struct st_t
{
    //int vertex[CITY_NUM * MAX_NEXT]; // TODO maybe extra , only once
    int path[CITY_NUM+1];
    //int stack[CITY_NUM+1];
    //int queue[CITY_NUM+1];
    int open[CITY_NUM+1];
    
    int step; // TODO steps in path
    float g; // g dist
    float h;
    float f;
};
typedef struct st_t st_t;

void print_path(int *path)
{
    printf("^^ : ");
    int i;
    for(i = 0; i < CITY_NUM + 1; i++)
        path[i]==NO_CITY ? printf("- "):printf("%d ", path[i]);
    printf("\n");    
}

void print_graph(void)
{
    int i;
    for(i = 0; i < CITY_NUM; i++)
    {
        printf("%d : ", i);

        for(int j = 0; j < MAX_NEXT; j++)
        {
            int index = i * MAX_NEXT + j;
            int cnum=g_vertex[index];
            cnum==NO_CITY? printf("-,"):printf("%d,", cnum);
        }
        printf("\n");
    }
}
void print_state(st_t *states_ptr)
{
    int i;
    printf("======\n");
    printf("f is %.1lf step is %d\n"
        ,(states_ptr)->f,(states_ptr)->step);
    
    
    
    //printf("......\n");
    
    printf("path : ");
    for(i = 0; i < CITY_NUM + 1; i++)
        (states_ptr)->path[i]==NO_CITY ? printf("- "):printf("%d ", (states_ptr)->path[i]);
    printf("\n");   
    
    printf("open : ");
    for(i = 0; i < CITY_NUM + 1; i++)
        (states_ptr)->open[i]==NO_CITY ? printf("- "):printf("%d ", (states_ptr)->open[i]);
    printf("\n");   

}

void path_keeper(int * path , enum RW rw)
{
    int i;
    static int flag = 0;
    static int *path_arr;

    if(flag == 0)
    {
        path_arr = calloc((CITY_NUM)+1, sizeof(int)); 
        flag = 1;
    }

    if(rw == READ)
        for(i=0; i<CITY_NUM+1; i++)
            path[i]=path_arr[i];
    else // WRITE
        for(i=0; i<CITY_NUM+1; i++)
            path_arr[i]=path[i];
    // TODO FREE
    return;
}


void states_keeper(st_t *state_ptr, enum RW rw) // TODO path
{
    //int ci;
    static st_t sk={0};
    
    if(rw==READ)
    {
        *state_ptr=sk;
    }
    else // WRITE
    {
        sk=*state_ptr;
    }
    
    //assert(si >= 0);
    //assert(si < MAX_STATES);
    /*if(rw == READ)
    {
        for(ci = 0; ci < CITY_NUM; ci++)
            nexts_keeper(&g_vertex[ci * MAX_NEXT], ci, READ);
        path_keeper((state)->path, READ);
    }
    else // WRITE
    {
        int nk[MAX_NEXT]; 
        for(ci = 0; ci < CITY_NUM; ci++)
        {
            nexts_keeper(nk, ci, READ);
            for(int j = 0; j < MAX_NEXT; j++) 
            {
                int index = ci * MAX_NEXT + j;
                g_vertex[index] = nk[j]; 
            }
        }
        //int pk[CITY_NUM+1];
        //path_keeper(pk,READ);
        
        //for(int j=0; j<CITY_NUM+1; j++)
          //  (state)->path[j]=pk[j];
        path_keeper((state)->path, WRITE);
    }*/
    return;
}
// TODO expand
// TODO add hash

void pre_search(int *vertices, int vcnt)
{
    int *nexts;
    int i;
    printf("vcnt is %d \n", vcnt);
    print_vertices(vertices, vcnt);
    for(i = 0; i < CITY_NUM; i++)
    {
        nexts = next_city(vertices, vcnt, i);
        printf("%d : nc[0] is %d nc[1] is %d sc is %d,%d,%d,%d nc[5] is %d\n", i, 
            nexts[0], nexts[1], nexts[2], nexts[3], nexts[4], nexts[5], nexts[6]);
        if(city_on_vertices(i, vertices, vcnt))
            nexts_keeper(nexts, i, WRITE);
        free(nexts);
    }
    
    // TODO compact nexts , sort
    printf("---\n");
    int nk[MAX_NEXT] = {-1};
    for(i = 0; i < CITY_NUM; i++)
    {
        printf("%d : ", i);
        nexts_keeper(nk, i, READ);
        qsort(nk, MAX_NEXT, sizeof(int), compare_ints);
        nexts_keeper(nk, i, WRITE);

        for(int j = 0; j < MAX_NEXT; j++)
        {
            if(nk[j]!=NO_CITY)
                printf("%d,", nk[j]);
            else
                printf("-,");
        }
        printf("\n");
    }
}

/*bool all_passed(int *const path)
{
    
    int i;
    if(path[0] != 0)
        return false;
    for(i = 1; i < CITY_NUM; i++)
        if(path[i] >= 0 && path[i] < CITY_NUM)
            continue;
    if(i == CITY_NUM && path[i] == 0)
        return true;
    return false;
}*/
bool all_passed(int *const path)
{
    
    int i;
    if(path[0] != 0)
        return false;
    for(i = 1; i < CITY_NUM; i++)
        if(path[i] == NO_CITY)
            return false;
    if(i == CITY_NUM && path[i] == 0)
        return true;
    return false;
}

bool all_nocity(void)//(st_t *state)
{
    int i;
    for(i = 0; i < CITY_NUM * MAX_NEXT; i++)
        if(g_vertex[i] != NO_CITY)
            return false;
    return true;
}
/// --- --- --- --- --- --- --- --- --- --- --- --- --- --- ///
int last_path(st_t * state_ptr);
bool adj(int r, int ind);
void multiple_open(st_t * state_ptr,int ind);
void neat_open(st_t * state_ptr);
void sort_open(st_t * state_ptr);

void remove_lp( st_t * state_ptr)
{
    int i;
    for(i=CITY_NUM-1; i>0; i--)
        if(state_ptr->path[i]!=NO_CITY)
        {
            state_ptr->path[i]=NO_CITY;
            break;
        }
    return;
}

//void match(st_t *states,int *si_ptr);
void remove_city(int ci, st_t * state_ptr)
{
    assert(ci != NO_CITY);
    int i;
    for(i=0; i<CITY_NUM; i++)
        if(state_ptr->open[i]==ci)
            state_ptr->open[i]=NO_CITY;
    return;
}

/*void remove_city(int city)//, st_t *state)
{
    int l;
    
    for( l=0; l<CITY_NUM*MAX_NEXT; l++)
    {
        if(g_vertex[l]==city )
        {
            g_vertex[l]=NO_CITY;
        }
    }
    
}*/

void eliminate(st_t * state_ptr)
{
    int j,l;
    
    for( j=1; (j<CITY_NUM+1) && state_ptr->path[j]!=NO_CITY; j++)
        for( l=0; l<CITY_NUM*MAX_NEXT; l++)
        {
            if(g_vertex[l]==state_ptr->path[j] )
                g_vertex[l]=NO_CITY;
        }
    
}

bool equal(st_t* st1_ptr, st_t* st2_ptr) // TODO compare path , stack
{
    //int l;
    //for(l=0; l<CITY_NUM*MAX_NEXT; l++)
      //  if(st1_ptr->vertex[l] != st2_ptr->vertex[l])
        //    return false;
    if(st1_ptr->step != st2_ptr->step)
    {
        fprintf(stderr,"vertices equal ,steps different:\n"); 
        print_state(st1_ptr);
        print_state(st2_ptr);
        exit(1);
    }
    
    // TODO dist
    return true;
}
bool equal_path(st_t* state1_ptr, st_t* state2_ptr)
{
    for(int i=0; i<CITY_NUM+1; i++)
        if(state1_ptr->path[i]!=state2_ptr->path[i])
            return false;
    return true;
}

bool on_path(int ci, st_t * state_ptr)
{
    //if(ci == NO_CITY)
      //  return true;
   // assert(ci != NO_CITY);
    int pi;
    for(pi=0; pi<CITY_NUM+1; pi++)
    {
        if(ci==state_ptr->path[pi])
            return true;
    }
    if(ci==0 && pi==CITY_NUM)
        return true;
    else
        return false;
}
bool on_open(int ci, st_t * state_ptr)
{
    //if(ci==NO_CITY)
      //  return true;
    //assert(ci != NO_CITY);
    int pi;
    for(pi=0; pi<CITY_NUM+1; pi++)
    {
        if(ci==state_ptr->open[pi])
            return true;
    }
    return false;
}

bool on_state(int city)//, st_t* state)
{
    bool r=false;
    for(int i=0; i<CITY_NUM*MAX_NEXT; i++)
        if(g_vertex[i]==city)
            r=true;
    return r;
}

bool possible_next(int next_city, st_t* state_ptr)
{
    
    print_state(state_ptr);
    
    if(next_city==0 && state_ptr->path[CITY_NUM-1]!=NO_CITY)
    {
        return true;
    }
    
    
    int j;
    
    for(j=0; j<MAX_NEXT; j++)
    {
        int index=next_city*MAX_NEXT+j;
        int g_city=g_vertex[index];
        if(g_city!=NO_CITY && !on_path(g_city,state_ptr) && on_open(g_city,state_ptr))
            return true;
    }
    return false;
}    
bool impossible(st_t * state_ptr)
{
    if(all_nocity())
        return true;
    
    if(state_ptr->path[1]==NO_CITY && state_ptr->open[0]!=NO_CITY)
        return false;
    
    int i;
    int lp=last_path(state_ptr);
    st_t sbu=*state_ptr;
    //remove_lp(state_ptr);
    
    //for(i=1; i<CITY_NUM; i++)
    for(i=0; state_ptr->open[i]!=NO_CITY; i++)
    {
        int soi=state_ptr->open[i];
        assert(soi!=NO_CITY);
        //if(possible_next(soi,state_ptr)&& adj(soi,lp) )// i
        if(!on_path(soi,state_ptr) && adj(soi,lp) )
            return false;
    }
    
    *state_ptr=sbu;
    return true;
}

int path_len(int * const path)
{
    int len;
    for(len=0 ;len<CITY_NUM+1 && path[len]!=NO_CITY ; len++)
        continue;
    len--;
    printf("len is %d\n",len);
    assert(len>=0);
    assert(len<CITY_NUM+1);
    
    return len;
}
bool complement(st_t * state_ptr)
{
    
    printf(" #$#$#$ \n");
    print_state(state_ptr);
    
    int i;
    
    for(i=0; i<CITY_NUM; i++)
    {
        if(on_path(i, state_ptr))
            continue;
        if(on_state(i))
            continue;
        return false;
    }
    return true;
}
/*
void match(st_t *states,int *si_ptr) // TODO ongoing
{
    int si=*si_ptr;
    st_t save_state=states[si];
    int i;
    
    while(all_nocity() && si>0) 
        si--;
    
    assert(si>=0);
    assert(si<= MAX_STATES);
        
    //for(i=*si_ptr-1; i>0; i--)
    for(i=0; i<*si_ptr-1; i++)
    {
        print_state(states);
        //assert(complement(states+i));
        if(equal_path(states+si,states+i) && !all_nocity()) // TODO not executed
        {
            *(states+*si_ptr+1)=*(states+i); // TODO overwrite , hold all states in memory
            print_state(states);
            
            break;
        }
    }
    if(i==0)
    {
        fprintf(stderr, " _+_+_+_+_ \n");
        print_state(states);
        print_state(&save_state);
        print_state(states);
        fprintf(stderr,"match/2 failed.\n");
        exit(1);
    }
    for(int l=0; l<CITY_NUM*MAX_NEXT; l++)
    {
        int city=g_vertex[l];
        if(city!=0 && city!=NO_CITY)
            assert( !on_path( city ,states+si) );
            
    }
    
    (*si_ptr)++;
    assert(complement(states+ *si_ptr));
}*/

// ....................................... //

/*void push(int ci, st_t *state)
{
    assert(state->stack[CITY_NUM]==NO_CITY);
    int i;
    for(i=0;i<=CITY_NUM; i++) // not already on stack 
        if(state->stack[i]==ci)
            return;
        
    for(i=CITY_NUM; i>0; i--)
        state->stack[i]=state->stack[i-1];
    state->stack[0]=ci;
}

int pop(st_t *state)
{
    int r=state->stack[0];
    assert(r!= NO_CITY);
    int i;
    for(i=1; i<=CITY_NUM; i++)
        state->stack[i-1]=state->stack[i];
    return r;
}
bool stack_is_empty(st_t *state)
{
    int i;
    for(i=0; i<=CITY_NUM; i++)
        if(state->stack[i]!= NO_CITY)
            return false;
    return true;
}*/
int last_path(st_t * state_ptr)
{
    int i;
    for(i=0; i<CITY_NUM; i++)
        if(state_ptr->path[i+1]==NO_CITY)
            return state_ptr->path[i];
    return NO_CITY;
}

void place(int ci, st_t * state_ptr)
{
    printf("---- entered place ----\n");
    assert(state_ptr->open[CITY_NUM]==NO_CITY);
    assert(ci != NO_CITY);
    int lp = last_path(state_ptr);
    int i;
     
    if( on_open(ci,state_ptr)|| on_path(ci,state_ptr))//state->queue[i]==ci
        return;
        
    for(i=0; i<CITY_NUM; i++)
        if(state_ptr->path[i]==NO_CITY)
            break;
    state_ptr->path[i]=ci;
    
    multiple_open(state_ptr,ci);
    
    city c0,c1,c2;
    city_info(&c0,0,READ);
    city_info(&c1,lp,READ);
    city_info(&c2,ci,READ);
    state_ptr->g += distance(c1,c2);
    
    state_ptr->h=distance(c0,c2); // TODO ->h 
    state_ptr->f=state_ptr->g +state_ptr->h ;
    state_ptr->step++;
    
    states_keeper(state_ptr,WRITE);
    print_state(state_ptr);
    printf("ci is %d\n",ci);
    printf("---- leave place ----\n");
}

bool adj(int r, int ind)
{
    if(r==NO_CITY || ind==NO_CITY)
        return true;
    int j;
    
    for(j=0; j<MAX_NEXT; j++)
    {
        int gvj=g_vertex[ind*MAX_NEXT+j];
        if(r==gvj)
            return true;
    }
    return false;
}

void improve_open(int extra ,st_t * state_ptr) 
{
    int i,j ;
    
    for(i=0; i<CITY_NUM; i++)
    {
        if(state_ptr->open[i]==extra)
        {
            state_ptr->open[i]=NO_CITY;
            for(j=i+1; j<CITY_NUM; j++)
                state_ptr->open[j-1]=state_ptr->open[j];
        }
    }
}
/*float h(int ci) // TODO try other heuristics
{
    if(ci==NO_CITY)
        return X_MAX+Y_MAX;//-10
    city cc,c0;
    city_info(&cc, ci, READ);
    city_info(&c0, 0, READ);
    float dist=distance(cc,c0);
    return dist;
    
}*/
/*float g(st_t const * const state)
{
    return state->g;
}
float f(st_t const * const state)
{
    return state->f;//g(state)+h(ci);
}*/

int compare(const void* a, const void* b)
{
    int arg1 = *(int*)a;
    int arg2 = *(int*)b;
 
    if(arg1==NO_CITY || arg2==NO_CITY)
        return 0;
    assert(arg1 != arg2);
 
    float f1;//=X_MAX+Y_MAX;//= .f;
    float f2;//=X_MAX+Y_MAX;
    st_t state_i;
    //states_keeper(&state0, 0, READ);
    city c0,c1,c2;
    city_info(&c0,0,READ);
    city_info(&c1,arg1,READ);
    city_info(&c2,arg2,READ);
    
    //int i;
    //for(i=0; i<MAX_STATES; i++)
    {
        //states_keeper(&state_i, i, READ);
        states_keeper(&state_i, READ);
        float g=state_i.g;
        int lp=last_path(&state_i);
        
        city lc;
        city_info(&lc,lp,READ);
        float g1 = g+distance(lc,c1);
        float g2 = g+distance(lc,c2);
        
        float h1= distance(c1,c0); // TODO better h ?
        float h2= distance(c2,c0);
        
        f1=g1+h1;
        f2=g2+h2;
        /*if(lp==arg1)
            f1=state_i.f;
        if(lp==arg2)
            f2=state_i.f;*/
    }
    
    //printf("compare %d %d f1 is %.1f f2 is %.1f\n" ,arg1,arg2,f1,f2);
    if (f1 < f2) return -1;
    if (f1 > f2) return 1;
    return 0;
    /*
    if(arg1.dist<arg2.dist) return -1;
    if(arg1.dist>arg1.dist) return 1;
    return 0;*/
} 
void sort_open(st_t * state_ptr)
{
    printf("++++ entered sort_open ++++\n");
    print_state(state_ptr);
    //float f_x[CITY_NUM+1];
    int i;
    //for(i=0; i<CITY_NUM+1; i++)
        //f_x[i]=f(state,state->open[i]);
    int open[CITY_NUM+1]={0}; // m init
    for(i=0; i<CITY_NUM+1; i++)
        open[i]=state_ptr->open[i];
        
    qsort(open, CITY_NUM+1, sizeof(int), compare);
    
    
    for( i=0; i<CITY_NUM+1; i++)
    {
        //printf("%.1f ",state->f);//f(state,state->open[i]));
        state_ptr->open[i]=open[i];
    }
    //states_keeper(state, 0, WRITE);
    print_state(state_ptr);
    printf("++++ leave sort_open ++++\n");
    //exit(1);
}
void fill_open(st_t * state_ptr,int ind)
{
    if(ind==0)
        return;
    int j=0;
    int k;
    while(state_ptr->open[j]!=NO_CITY)
        j++;
    for(k=j; (k<MAX_NEXT+j); k++)//CITY_NUM MAX_NEXT
    {
        int gv=g_vertex[ind*MAX_NEXT+k-j];
        if(on_path(gv,state_ptr)|| on_open(gv,state_ptr))
            continue;
        state_ptr->open[k]=gv;
    }
    
}
void neat_open(st_t * state_ptr)
{
    int i,j;
    for(i=0; i<CITY_NUM; i++)
        if(state_ptr->open[i]==NO_CITY )//&&  state_ptr->open[i]!=0
            for(j=i+1; j<=CITY_NUM ; j++)
            //for(j=CITY_NUM; j>0 ; j--)
                if(state_ptr->open[j]!=NO_CITY )//&& state_ptr->open[j]!=0
                {
                    state_ptr->open[i]=state_ptr->open[j];
                    state_ptr->open[j]=NO_CITY;
                    break;
                }
}
void multiple_open(st_t * state_ptr,int ind)
{
    //(void)ind;
    fill_open(state_ptr,ind);
    neat_open(state_ptr);
    sort_open(state_ptr);
}

int banish(st_t * state_ptr) // TODO seems a problem here
{
    printf("@@@ entered banish @@@\n");
    print_state(state_ptr);
    
    int r=0; // m init
    int i;
    int lp=last_path(state_ptr);
    //float f_x=X_MAX+Y_MAX;
    //for(i=0;i<CITY_NUM;i++)
    // multiple_open
    //fill_open(state_ptr,lp);
    neat_open(state_ptr);
    sort_open(state_ptr);
    
    //for(i=CITY_NUM;i>0;i--)
    for(i=0;i<CITY_NUM;i++)
    {
        r=state_ptr->open[i];
        if(r==NO_CITY)
            continue;
        
        if( on_path(r,state_ptr) || /*on_open(r,state_ptr) ||*/ !adj(r,lp) )//|| !adj(r,lp))
            continue;
            
        else//(r==NO_CITY  )
        {
            
            r=state_ptr->open[i];
            break;
        }
    }
    //i=0;
    /*while(!adj(r,lp))
    {
        if(i>CITY_NUM) // TODO how to make progress ?
        {
            //state->queue[0]=NO_CITY;
            
            return NO_CITY;
            //break;
            //exit(1);
        }
        r=state->open[i];
        i++;
    }*/
    //r=state->open[i];
    int j=0;
    printf("i is %d , r is %d\n",i,r);
    /*if(r==NO_CITY) // TODO go back , eliminate last choice , continue 
    {
        //state->path[6]=NO_CITY;
        //state->open[0]=NO_CITY;
        //lp = last_path(state);
         //state--;
         st_t os;
         states_keeper(&os,0,READ);
        
        improve_open(lp,state);
        for(j=0; j<CITY_NUM; j++)
            if(os.path[j]==lp)
                os.path[j]=NO_CITY;
        //int lp2=last_path(&os);
        //fill_open(&os,lp2);
        //sort_open(&os);
        
        states_keeper(&os,0,WRITE);
        print_state(&os,0);
        exit(1);
        //deque(state);
        //int ci=state->open[0];
        //enque(lp2,state);
    }*/
    assert(r!=NO_CITY);
    
    j=0;
    
    assert(r!= NO_CITY);
    
    /*for(i=1; i<=CITY_NUM ; i++)
    {
        if(state->queue[i-1]!=NO_CITY )
            continue;
        if(on_path(i,state) && on_queue(i,state))
            continue;
        
        for(j=CITY_NUM; j>=i;j--)
        {
            if(on_queue(r,state) &&state->queue[j]==r )
            {
                state->queue[j]=NO_CITY;       
            }   
        }
        state->queue[i-1]=state->queue[i];
    }*/
    printf("r is %d i is %d j is %d lp is %d\n",r,i,j,lp);
    
    /*for(i=0; i<CITY_NUM; i++)
    {
        if(state->queue[i]==r)
        {
            state->queue[i]=NO_CITY; 
            for(j=CITY_NUM; j>0; j--)
            {
                //state->queue[j-1]=state->queue[j];
                
            }
            
        }
    }*/
    
    //if(state->queue[0]==NO_CITY)
      //  for(i=1; i<=CITY_NUM ; i++)
        //    state->queue[i-1]=state->queue[i];
    improve_open(r,state_ptr);
    //neat_open(state_ptr);
    //sort_open(state_ptr);
    assert(!on_path(r,state_ptr));
    
    state_ptr->step++;
    states_keeper(state_ptr,WRITE);
    print_state(state_ptr);
    printf("@@@ leaving banish @@@\n");
    //exit(1);
    return r;
}
bool open_is_empty(st_t * state_ptr)
{
    int i;
    for(i=0; i<=CITY_NUM; i++)
        if(state_ptr->open[i]!= NO_CITY)
            return false;
    return true;
}

bool path_is_full(st_t *state) // TODO distance , etc
{
    int i;
    for(i=0; i<=CITY_NUM; i++)
        if(state->path[i]==NO_CITY)
            return false;
    return true;
}

bool full_but_last(st_t * state_ptr)
{
    if(state_ptr->path[CITY_NUM-1]!=NO_CITY)
        return false;
    int i;
    for(i=0; i<CITY_NUM-1; i++)
        if(state_ptr->path[i]==NO_CITY)
            return false;
    return true;
}

st_t init(void)
{
    st_t state={0};
    int ci;
    int nk[MAX_NEXT]; 
    int i;
    
    state.path[0]=0;
    for(i=1; i<CITY_NUM+1; i++)
    {
        state.path[i]=NO_CITY;
        state.open[i]=NO_CITY;
    }
    state.path[CITY_NUM]=0;
    
    for(ci = 0; ci < CITY_NUM; ci++)
    {
        nexts_keeper(nk, ci, READ);
        for(int j = 0; j < MAX_NEXT; j++) 
        {
            int index = ci * MAX_NEXT + j;
            g_vertex[index] = nk[j]; 
        }
    }
    
    for(i=0; i<MAX_NEXT-1; i++)
        state.open[i]=g_vertex[i];
        
        //int pk[CITY_NUM+1];
        //path_keeper(pk,READ);
        
        //for(int j=0; j<CITY_NUM+1; j++)
          //  (state)->path[j]=pk[j];
    path_keeper((state).path, WRITE);
    return state;
}

//    ======================================    //
bool full_path(int * path);

void swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

int compare_ints_rev(const void* a, const void* b)
{
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
 
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}
bool path_is_unique(int * path)
{
    bool r=true;
    int i;
    int * path2 = malloc((CITY_NUM+1)*sizeof(int));
    for(i=0; i<CITY_NUM+1; i++)
        path2[i]=path[i];
    
    print_path(path2);
    qsort(path2, CITY_NUM+1, sizeof(int), compare_ints_rev);
    if(path2[0]!=0 || path2[1]!=0)
        r= false;
    for(i=2; i<CITY_NUM+1; i++)
        if(path2[i]!=i-1)
            r= false;
    print_path(path2);
    free(path2);
    return r;
}

float path_dist(int * path)
{
    float dis=0.0;
    int i;
    city from,to;
    assert(path[0]==0);
    assert(path[CITY_NUM]==0);
    //assert(full_path(path));
    print_path(path);
    
    for(i=0; i<CITY_NUM; i++)
    {
        city_info(&from,path[i],READ);
        city_info(&to, path[i+1], READ);
        //assert(from.next_i==path[i+1]); // TODO maybe return -1.0
        
        dis += distance(from,to);
    }
    assert(path[i+1]==0);
    return dis;
}
// ============================ //
bool full_path(int * path)
{
    int i;
    city ci;
    assert(path[0]==0);
    assert(path[CITY_NUM]==0);
    print_path(path);
    for(i=0; i<CITY_NUM; i++)
    {
        city_info(&ci, path[i], READ);
        if(ci.next_i != path[i+1])
            return false;
    }
    //assert(path_is_unique(path));
    if(!path_is_unique(path))
        return false;
    return true;
}
int * make_path(void)
{
    int * new_path=calloc(CITY_NUM+2, sizeof(int));
    int i;
    city ci;
    new_path[0]=0;
    city_info(&ci, 0, READ);
    for(i=1; i<CITY_NUM; i++)
    {
        int next=ci.next_i;
        city_info(&ci, next, READ);
        new_path[i]=next;
    }
    assert(new_path[0]==0);
    assert(new_path[CITY_NUM]==0);
    //assert(path_is_unique(new_path));
    
    print_path(new_path);
    return new_path;
}
// +++++++++++++++++++++++++++++ //

bool same_path(int * path1, int * path2)
{
    int i;
    for(i=0; i<CITY_NUM+1; i++)
        if(path1[i]!=path2[i])
        {
            printf("path1[%d] is %d path2[%d] is %d \n",i,path1[i],i,path2[i]);
            return false;
        }
    return true;
}

// ============================= //
void path_to_cities(int * path)
{
    int i;
    city ci;
    //assert(full_path(path));
    for(i=0; 1; i++)//i<CITY_NUM
    {
        city_info(&ci, path[i], READ);
        ci.next_i=path[i+1];
        city_info(&ci, path[i], WRITE);
        if(path[i+1]==0)
            break;
    }
}

void rev_nexts( int beg, int end) // TODO ongoing:
{ 
    city bc,ec;
    city ci;
    int i;
    int * part = calloc(CITY_NUM+2, sizeof(int));
    
    if(beg==end)
        return;
    
    //city_info(&ci, beg, READ);
    city_info(&ci, end, READ);
    city_info(&bc, beg, READ);
    city_info(&ec, end, READ);
    if(same_city(bc, ec))
        return;
    
    part[0]=end;//beg;
    for(i=1; i<CITY_NUM+1; i++) // fill "part"
    {
        if(part[i]==beg)//end
            break;
        if(same_city(ci, bc))//ec
            break;
        part[i] = ci.next_i;
        city_info(&ci, ci.next_i, READ); 
    }
    //part[i]=beg;//end;
    i--;
    printf("\n$@$@ \n");
    //printf("beg: %d end is %d i is %d :\n",beg,end,i);
    for(int k=0; k<=i; k++)
    {
        printf("%d->",part[k]);
    }
    printf("\n");
    
    // TODO maybe acbd here
    
    int j;
    int * path = calloc(CITY_NUM+2, sizeof(int));
    //int * path=make_path();
    int * orig_path=make_path();
    for(j=0; j<CITY_NUM; j++) // stage 1
    {
        if(orig_path[j]==end || orig_path[j]==beg )
            break;
        path[j]=orig_path[j];
    }
    //j++;
    path[j]=beg;
    j++;
    //j=CITY_NUM-j;
    //i=CITY_NUM-i;
    i--;
    printf("\n## beg: %d end is %d i is %d j is %d path[i] is %d path[j] is %d:\n",beg,end,i,j,path[i],path[j]);
    print_path(path);
    while(j<CITY_NUM) // stage 2
    {
        path[j]=part[i];
        //if(part[i]==beg || part[i]==end)
        if(i==0)
            break;
        j++;
        i--;
    }
    print_path(path);
    for( j++; true; j++) // stage 3
    {
        if(path[j]==0)
            break;
        path[j]=orig_path[j];
    }
    
    print_path(path);
    path_to_cities(path);
    /*
    i=CITY_NUM-i; // CITY_NUM-i
    for(; i>=1; i--) // reverse beg..end : end..beg
    { // TODO correct offset
        assert(i>0);
        assert(i<=CITY_NUM+1);
        city_info(&ci, part[i], READ);
        ci.next_i=part[i-1];
        printf("%d:%d ",i-1,part[i-1]);
        city_info(&ci, part[i], WRITE); 
    }*/
    // TODO use path_to_cities/1
    
    printf("\n");
    int * new_path=make_path();
    print_path(new_path);
    printf("$$@@\n\n");
    free(part);
    return;
}

void undo_rev_nexts( int beg, int end, int p_a, int p_b, int p_c, int p_d) // TODO seems extra
{
    assert(beg != end);
    city ca,cc;
    //city bb;
    
    printf("\n*#*#\n");
    int * path1=make_path();
    print_path(path1);
    
    
    rev_nexts(beg, end);
    city_info(&ca, p_a, READ);
    ca.next_i=p_b;
    city_info(&ca, p_a, WRITE);
    city_info(&cc, p_c, READ);
    cc.next_i=p_d;
    city_info(&cc, p_c, WRITE);
    
    //rev_nexts(beg, end);
    
    int * path2=make_path();
    print_path(path2);
    printf("_*#*#\n\n");
    assert(full_path(path2));
    return;
}
void acbd_path(city *cij, int p_a, int p_b, int p_c, int p_d) // a->c b->d
{ // TODO maybe changes cij
    city cc[4]={cij[0],cij[1],cij[2],cij[3]};
    cc[0].next_i=p_c;
    cc[1].next_i=p_d;
    city_info(&cc[0], p_a, WRITE);
    city_info(&cc[1], p_b, WRITE);
}
void reset_abcd(city * cc,int p_a, int p_b, int p_c, int p_d) 
{
    city_info(&cc[0], p_a, WRITE);
    city_info(&cc[1], p_b, WRITE);
    city_info(&cc[2], p_c, WRITE);
    city_info(&cc[3], p_d, WRITE);
}
bool record_path_dist(int * path)
{
    float min_dist, curr_dist;
    low_dist(&min_dist,READ);
    curr_dist=path_dist(path);
    if(curr_dist<min_dist)
    {
        path_keeper(path, WRITE);
        low_dist(&curr_dist,WRITE);
        return true;
    }
    return false;
}

bool single_uncross(int * path, city *cij,int a, int b, int c, int d) // TODO try other possibilities like c->a d->b
{
    assert(path!=NULL);
    assert(full_path(path));
    assert(path[0]==0);
    assert(path[CITY_NUM]==0);
    assert(path_is_unique(path));
    
    //bool r=false;
    int * new_path;
    bool fp;
    //city cc[4]={cij[0],cij[1],cij[2],cij[3]};
    
    int *path1=make_path();
    printf("&& :\n");
    new_path=make_path();
    assert(same_path(new_path, path1));
    print_path(path);
    printf("____________________________________________________\n");
    if(1) // TODO for debugging
    {
        rev_nexts(path[d], path[a]);
        //undo_rev_nexts(path[a], path[d], path[a], path[b], path[c], path[d]); // reset
        path_to_cities(path1);
        int *path0=make_path();
        assert(same_path(path0, path1)); // also for b <-> c
        rev_nexts(path[b], path[c]);
        //undo_rev_nexts(path[c], path[b], path[a], path[b], path[c], path[d]); // reset
        path_to_cities(path1);
        int *path4=make_path();
        assert(same_path(path4, path1));
    }
    
    acbd_path(&cij[0], path[a], path[b], path[c], path[d]);
    new_path=make_path();
    fp= full_path(new_path);
    if(fp==true)
    {
        // TODO dist , path
        bool rpd=record_path_dist(new_path);
        if(rpd==true)
        {
            path_to_cities(new_path);
            return true;
        }
    }
    
    printf("++ :\n");
    /*city bc,ec;
    rev_nexts(path[d], path[a]); // or path[b] path[c]
    city_info(&bc, path[d], READ);
    city_info(&ec, path[a], READ);
    bc.next_i = ec.next_i;
    city_info(&bc, path[d], WRITE);
    for(int j=0; j<= CITY_NUM; j++)
    {
        city cj;
        city_info(&cj, j, READ);
        if(cj.next_i==path[d])
        {
            cj.next_i=path[a];
            city_info(&cj, j, WRITE); // edge case
            break;
        }
    }*/
    path_to_cities(path1);
    rev_nexts(path[d], path[a]);
    acbd_path(&cij[0], path[a], path[b], path[c], path[d]);
    
    new_path=make_path();
    print_path(new_path);
    fp= full_path(new_path);
    if(fp==true)
    {
        // TODO dist , path
        bool rpd=record_path_dist(new_path);
        if(rpd==true)
        {
            path_to_cities(new_path);
            return true;
        }
    }
    
    //rev_nexts(path[a], path[d]); // reset
    
    //undo_rev_nexts(path[a], path[d], path[a], path[b], path[c], path[d]);
    //reset_abcd(&cc[0], path[a], path[b], path[c], path[d]);
    //int *path2=make_path();
    
    path_to_cities(path1);
    rev_nexts(path[b], path[c]);
    acbd_path(&cij[0], path[a], path[b], path[c], path[d]);
    
    printf("+- :\n");
    new_path=make_path();
    fp= full_path(new_path);
    if(fp==true)
    {
        // TODO dist , path
        bool rpd=record_path_dist(new_path);
        if(rpd==true)
        {
            path_to_cities(new_path);
            return true;
        }
    }
    
    path_to_cities(path1);
    rev_nexts(path[c], path[b]);
    acbd_path(&cij[0], path[a], path[b], path[c], path[d]);
    
    printf("-+ :\n");
    new_path=make_path();
    fp= full_path(new_path);
    if(fp==true)
    {
        // TODO dist , path
        bool rpd=record_path_dist(new_path);
        if(rpd==true)
        {
            path_to_cities(new_path);
            return true;
        }
    }
    
    //rev_nexts(path[c], path[b]); // reset
    
    //undo_rev_nexts(path[c], path[b],path[a], path[b], path[c], path[d]);
    //reset_abcd(&cc[0], path[a], path[b], path[c], path[d]);
    path_to_cities(path1);
    
    int *path3=make_path();
    //assert(same_path(path1, path2));
    //assert(same_path(path2, path3));
    assert(same_path(path1, path3));
    // TODO if(r==true)
    return false;    
}

bool alt_path(int * path, city *cij, int a, int b, int c, int d)// original : a->b c->d
{ 
    assert(path!=NULL);
    assert(full_path(path));
    assert(path[0]==0);
    assert(path[CITY_NUM]==0);
    assert(path_is_unique(path));
    
    
    // TODO turn into a single function:
    return single_uncross(path, cij, a, b, c, d);
    // a->c b->d
    //single_uncross(path, cij, a, c, b, d);
    
    // TODO call rev_nexts/2 to be back again
    // c->a d->b
    
}
bool post_search(int * path) // correct line segments crosses
{ // TODO correct min_dist , cities array , path
    int i,j;
    city cij[4];
    //city cc[4];
    assert(path!=NULL);
    int * new_path=calloc(CITY_NUM + 2, sizeof(int));
    //int * new_path2=calloc(CITY_NUM + 1, sizeof(int));
    for(i=0; i<CITY_NUM; i++)
    {
        new_path[i]=path[i];
        //new_path2[i]=path[i];
    }
    //float p_dis=path_dist(path);
    
    assert(full_path(path));
    assert(path[0]==0);
    assert(path[CITY_NUM]==0);
    
    for(i=0; i<CITY_NUM-1; i++)
        for(j=i+1; j<CITY_NUM; j++) // i i+1 >< j j+1
        {
            city_info(&cij[0], path[i], READ);
            city_info(&cij[1], path[i+1], READ);
            city_info(&cij[2], path[j], READ);
            city_info(&cij[3], path[j+1], READ);
            
            if(lines_cross(cij[0],cij[1],cij[2],cij[3])) // TODO two possible changes
            {
                assert(full_path(path));
                // TODO try 4 * 2 possibilities
                // TODO multiple functions
                
                // 0 2 , 1 3
                /*bool aw1=alt_ways(path, new_path, &cij[0], i, j, i+1, j+1); // TODO remove these
                bool aw2=alt_ways(path, new_path, &cij[0], i, j, j+1, i+1);
                bool aw3=alt_ways(path, new_path, &cij[0], j, i, i+1, j+1);
                bool aw4=alt_ways(path, new_path, &cij[0], j, i, j+1, i+1);
                assert(aw1 || aw2 || aw3 || aw4);*/
                // 0 3 , 1 2
                bool ap=alt_path(path, &cij[0], i, i+1, j, j+1);
                
                assert(ap==true);
                //path_keeper(new_path, READ);
                //assert(full_path(new_path));
                //return new_path;
                //return path;
                //continue; // TODO 
                path_keeper(path, READ);
                return false;
            }
        }
    
    //return new_path;
    return true;
}

void A_star_algorithm(void) // TODO use another way to find all solutions
{// Breadth first search
    int i,max_i=0;
    int best_changed=0;
    
    remove_repetition();
    print_keeper();

    st_t *all_states = calloc(MAX_STATES+1, sizeof(st_t)); // TODO single state ?
    st_t *state_ptr = calloc(1, sizeof(st_t));
    
    float min_dist;
    distance_keeper(&min_dist, READ);
    int path0[CITY_NUM+1]={NO_CITY};
    path0[0]=0;
    
    for(i=1; i<CITY_NUM+1; i++)
        path0[i]=NO_CITY;
    path0[CITY_NUM]=0;//0; // recently added
    path_keeper(path0, WRITE);
    path_keeper(path0,READ);
    print_path(path0);

    printf("++++++++++++++++++++++++++++++++++++++++ ++++++++++++++++++++++++++++++++++++++++\n");
    printf("min_dist is %lf\n", min_dist);

    *state_ptr=init();
    states_keeper(state_ptr, WRITE);
    
    for(i=0; i<CITY_NUM+1; i++)
        state_ptr->open[i]=NO_CITY;//stack
    for(i=0; i<CITY_NUM+1; i++)
    {
        if(g_vertex[i]==NO_CITY)
        {   
            break;
        }
        state_ptr->open[i]=g_vertex[i];// stack
        //states[0].stack[i]=NO_CITY; // init stack
    }
    
    
    states_keeper(state_ptr, READ);
    print_state(state_ptr);
    //print_state(states, 1);
    all_states[0]=*state_ptr;
    
    int *best_path = calloc(CITY_NUM + 1, sizeof(int));
    int next = 0;
    for(i = 0; i < CITY_NUM + 1; i++)
    {
        city ci;
        city_info(&ci, next, READ);
        best_path[i] = next; 
        next = ci.next_i;
    }
    best_path[CITY_NUM] = 0;
    print_path(best_path);
    print_graph();
    
    int sol1_flag=0;
    int solutions=0;
    // TODO total distance(later min dis) is a cap
    // TODO stop on distance cap or any remaining city with no choice
    for(i = 1; i < MAX_STATES; i++) // TODO success forward backward
    {
        all_states[i]=*state_ptr;
        
        //assert(complement(state_ptr));
        if(i>max_i)
            max_i=i;
        
        print_state(state_ptr);
        
        assert(complement(state_ptr));
        
        int top=NO_CITY;
        
        int lp=last_path(state_ptr);
        
        if(sol1_flag==0 ) // TODO ongoing
        { 
            if(!impossible(state_ptr))
            {
                top=banish(state_ptr);//pop
            }
            else 
            {
                print_state(state_ptr);
                while(impossible(all_states+i))
                {
                    
                    i--;
                }
                if(i<0)
                    break;
                
                remove_city(lp,all_states+i);
                
                multiple_open(all_states+i,lp);
                
                *state_ptr=all_states[i];
                print_state(state_ptr);
                
            }
            
        }
        else if (open_is_empty(state_ptr))
        {
            while(open_is_empty(all_states+i))
                i--;
            if(i<0)
                break;
            
            all_states[i].open[0]=NO_CITY;
            neat_open(all_states+i);
            *state_ptr=all_states[i];
            
            //continue;
        }
        else if(adj(state_ptr->open[0] ,lp)) // TODO this seems like a work-around
        {
            
            top=state_ptr->open[0];
            
            state_ptr->open[0]=NO_CITY;
            neat_open(state_ptr);
            place(top,state_ptr);
            states_keeper(state_ptr, WRITE);
            
            multiple_open(state_ptr,top);
            
            print_state(state_ptr);
            
        }else if(!adj(state_ptr->open[0] ,lp))
        {
            
            state_ptr->open[0]=NO_CITY;
            neat_open(state_ptr);
            
        }  
        
        int j;
        
        
        // TODO step 3:
          
        int ci=top; // TODO correct above
                
        bool op=on_path( ci,state_ptr);
        if( ci != NO_CITY && !op)
        {
            place(ci, state_ptr);//push
            states_keeper(state_ptr, WRITE);
                
        }
        
        if(full_but_last(state_ptr))
        {
            print_state(state_ptr);
            int ci[CITY_NUM + 1];
            int i2=i;
            int lr=NO_CITY;
            for(i=0;i<CITY_NUM ; i++)
                ci[i]=i;
            for(i=0; i<CITY_NUM ; i++)
                ci[state_ptr->path[i]]=NO_CITY;
            for(i=0; i<CITY_NUM ; i++)
                if(ci[i]!=NO_CITY)
                    lr=ci[i];
            assert(lr != NO_CITY);
            assert(!on_path(lr,state_ptr));
            state_ptr->path[CITY_NUM-1]=lr;
            state_ptr->open[0]=NO_CITY;
            assert(path_is_full(state_ptr));
            print_state(state_ptr);
            printf("lr is %d\n",lr);
            // TODO add distance
            city cbl,cl,c0;
            city_info(&cbl,state_ptr->path[CITY_NUM-2],READ);
            city_info(&cl,state_ptr->path[CITY_NUM-1],READ);
            city_info(&c0,state_ptr->path[CITY_NUM],READ);
            state_ptr->g += distance(cbl,cl);
            state_ptr->g += distance(cl,c0);
            state_ptr->f = state_ptr->g;
            state_ptr->h = 0.0;
            
            i=i2;
        }
        
        if(path_is_full(state_ptr))
        {
            solutions++;
            float dist = state_ptr->g;
            float curr_dist;
            low_dist(&curr_dist,READ);
            if(curr_dist> (9.0/10.0)*FLT_MAX)
                low_dist(&dist,WRITE);
        }
        
        if(path_is_full(state_ptr) && state_ptr->g<min_dist) // better solution found
        {
            best_changed++;
            min_dist=state_ptr->g;
            
            low_dist(&min_dist, WRITE);
            for( j = 0; j < CITY_NUM + 1; j++)
                best_path[j] = (state_ptr)->path[j];
        }
        
        int end_flag=0;
        if((open_is_empty(state_ptr) )&& !sol1_flag) // first solution
        { 
            sol1_flag=1;
            float curr_dist=state_ptr->g;
            low_dist(&curr_dist,WRITE);
            
            while(1) // TODO how to continue for more solutions ?
            
            {
                
                for(j=CITY_NUM-1; state_ptr->path[j]==NO_CITY; j--)
                    continue;
                
                if(j<=0)
                {
                    end_flag=1;
                    break;
                }
            
                lp = state_ptr->path[j];
                assert(lp!=NO_CITY);
                printf("lp is %d j is %d\n",lp,j);
            
                int k=j;
                int i2=i;
                int inside_flag=0;
                
                for( i2=i; i2>=0 && j>=0 ; i2--) // TODO modify previous states
                {
                    for( j=k-1; j>0 ; j--)
                    {
                    
                        int blp=all_states[i2].path[j];
                    
                        if(blp==NO_CITY)
                            continue;
                        assert(blp != NO_CITY);
                    
                        assert(lp!=blp);
                        if( (adj(lp,blp) || adj(blp,lp)) &&(k-j>1)) //
                        {
                            inside_flag=1;
                            break;
                        }
                        
                        remove_city(lp,all_states+i2);//
                        
                        neat_open(all_states+i2);
                        sort_open(all_states+i2);
                        *state_ptr=all_states[i2];
                    }
                    if(inside_flag)
                        break;
                }
                if(inside_flag)
                    break;
            
                *state_ptr=all_states[i];
                print_state(state_ptr);
            
                if(i<=0)
                {
                    end_flag=1;
                    break;
                }   
            
                if(!impossible(all_states+i))
                    break;
                i--;    
            }
        }
        
        //if(path_is_full(state_ptr)   ) // TODO stop at same path repetition
        if(end_flag)
        {   
            
            print_state(state_ptr);
            break;
        }    
        states_keeper(state_ptr, WRITE);
        
    }
    
    for(int j=0; j< CITY_NUM; j++)
    {
        city cj;
        city_info(&cj, best_path[j], READ);
        
        cj.next_i=best_path[j+1];
        city_info(&cj, best_path[j], WRITE);
    }
    
    for(int j=0; j< CITY_NUM; j++)
    {
        city cj;
        city_info(&cj, j, READ);
    }
    printf("\n");
    
    if(!sol1_flag)
        low_dist(&min_dist,WRITE);
    
    print_path(best_path);
    assert(path_is_unique(best_path));
    int *path=best_path;
    //int* new_path;
    while(1) // new code
    {
        //new_path=post_search(path);
        bool finished=post_search(path);
        if(finished)
            break;
        /*for(int j=0; j<=CITY_NUM; j++)
        {
            if(new_path[j] != path[j])
            {
                path=new_path;
                continue;
            }
        }
        */
        //if(same_path(path, new_path))
            //break;
        path_keeper(path, READ);
    }
    print_path(path);
    assert(path_is_unique(path));
    assert(full_path(path));
    for(int j=0; j<CITY_NUM; j++)
    {
        city cj;
        best_path[j]=path[j];
        city_info(&cj, path[j], READ);
        assert(cj.next_i==path[j+1]);
    }
    best_path[CITY_NUM]=0;
    low_dist(&min_dist,READ);
    assert(path_is_unique(best_path));
    //free(path);
    //free(new_path);
    
    printf("min_dist is %.1lf max_i is %d i is %d bc is %d solutions: %d s1 is %d\n"
        , min_dist,max_i,i,best_changed,solutions,sol1_flag);
    print_path(best_path);

    free(best_path);
    free(all_states);
    free(state_ptr);
    return;
}

// ************************************************ //
// TODO draw cycle
void show_cycle2(SDL_Renderer *renderer, int on_cycle)
{
    int i, j;
    int base_0, base_i, base_j;
    city c1, c2, c3;

    cycle_keeper(&base_0, 0, on_cycle, READ);
    city_info(&c1, base_0, READ);

    for(i = 1; i < CITY_NUM - 1; i++)
    {
        cycle_keeper(&base_i, i, on_cycle, READ);
        city_info(&c2, base_i, READ);

        for(j = i + 1; j < CITY_NUM; j++)
        {
            cycle_keeper(&base_j, j, on_cycle, READ);
            city_info(&c3, base_j, READ);

            //S2D_DrawTriangle(c1.x, c1.y, 0, 0, 0, 0.1, c2.x, c2.y, 0, 0, 0, 0.1, c3.x, c3.y, 0, 0, 0, 0.1);
            SDL_Vertex vertices[]=
            {
                { {c1.x, c1.y}, pol, { 0,0 } },// { 255, 0, 0, 255 }
                { {c2.x, c2.y}, pol, { 0,0 } },
                { {c3.x, c3.y}, pol, { 0,0 }}
            };
            SDL_RenderGeometry(renderer,NULL, vertices,3,NULL,0);
            
            break;
        }
    }
}

void show_cycle(SDL_Renderer *renderer,int on_cycle) 
{
    int i, j, base_i;

    for(i = 0; i < CITY_NUM; i++)
    {
        city base;
        city_info(&base, i, READ);
        if(base.on_cycle == on_cycle)
        {
            base_i = i;
            break;
        }
    }
    if(i == CITY_NUM)
        return;
    
    city c1, c2, c3;
    city_info(&c1, base_i, READ); 

    for(i = 0; i < CITY_NUM - 1; i++)
    {

        city_info(&c2, i, READ); 
        
        if(c2.on_cycle != on_cycle)
            continue;

        if(base_i == i /*|| base_i == j || i == j*/)
            continue;
        for(j = i + 1; j < CITY_NUM; j++)
        {
            city_info(&c2, i, READ);

            city_info(&c3, j, READ); 
            
            if(base_i == j || i == j)
                continue;
            if(c3.on_cycle != on_cycle)
                continue;

            //S2D_DrawTriangle(c1.x, c1.y, 0, 0, 0, 0.1, c2.x, c2.y, 0, 0, 0, 0.1, c3.x, c3.y, 0, 0, 0, 0.1);
            SDL_Vertex vertices[]=
            {
                { {c1.x, c1.y}, pol, { 0,0 } },// { 255, 0, 0, 255 }
                { {c2.x, c2.y}, pol, { 0,0 } },
                { {c3.x, c3.y}, pol, { 0,0 }}
            };
            SDL_RenderGeometry(renderer,NULL, vertices,3,NULL,0);
            
        }
    }
}

// ------------------------------------------------------------------------------------//
void render_text(SDL_Renderer ** renderer,TTF_Font *font,Sint16 x, Sint16 y, char * text)
{
    
    assert(strlen(text) <= 80);
    
    SDL_Surface * ts = TTF_RenderUTF8_Blended(font, text, black);
    SDL_Texture* tx=SDL_CreateTextureFromSurface(*renderer,ts);
    int width = strlen(text)*8;
    SDL_Rect rec={.x=x-20, .y=y-20, .w=width, .h=15};
    SDL_RenderCopy(*renderer, tx, NULL, &rec);
}
/*
void render_text(SDL_Renderer * renderer,TTF_Font *font,Sint16 x, Sint16 y, char * text)
{
    
    //gfxPrimitivesSetFont( ,20,20);
    SDL_Surface * ts = TTF_RenderUTF8_Blended(font, text, white);
    SDL_Texture* tx=SDL_CreateTextureFromSurface(renderer,ts);
    
    SDL_Rect rec={.x=x-10, .y=y-10, .w=30, .h=20};
    SDL_RenderCopy(renderer, tx, NULL, &rec);//&src, &dst)
    //stringColor(renderer, x, y, text, 0xFFAABBCC); 
}*/

void render_sub(SDL_Renderer **renderer,TTF_Font *font, double time_elapsed)
{
    char text[80]={'\0'};
    char ts[80]={'\0'};
    float dis;
    low_dist(&dis, READ);
    
    snprintf(text, 79,"Total distance is %.1lf cities_count is %d", dis, CITY_NUM); 
    printf("%s\n",text);
    /*
    SDL_Surface * ts = TTF_RenderUTF8_Blended(font, text, black);
    SDL_Texture* tx=SDL_CreateTextureFromSurface(*renderer,ts);
    int width = strlen(text)*8;
    assert(width<X_MAX);
    SDL_Rect rec={.x=50, .y=Y_MAX-50, .w=width, .h=15};
    SDL_RenderCopy(*renderer, tx, NULL, &rec);
    */
    snprintf(ts, 79, "Time elapsed on solving is %.2lf sec",time_elapsed);
    printf("%s\n",ts);
    
    render_text(renderer,font,50,Y_MAX-50,text);
    render_text(renderer,font, 50, Y_MAX-25,ts);
}


void render(SDL_Renderer **renderer,TTF_Font *font, double time_elapsed) 
{
    
    city start_end;
    city_info(&start_end, 0, READ); // begin from here
    float x = start_end.x;
    float y = start_end.y;
    float dis0 = 0;
    distance_keeper(&dis0, WRITE);

    //S2D_DrawCircle(x, y, radius, sectors, 0.0, 0.0, 1.0, 1.0);
    filledCircleColor(*renderer,  x,  y, 9, 0xFF0000FF);

    /*S2D_Text *beg_text = S2D_CreateText("/usr/share/fonts/gnu-free/FreeSans.ttf", start_end.name, 15);
    beg_text->x = x + 10;
    beg_text->y = y;
    beg_text->color.r = 0.1;
    beg_text->color.g = 0.1;
    beg_text->color.b = 0.1;
    beg_text->color.a = 1.0;
    S2D_DrawText(beg_text);
    S2D_FreeText(beg_text);*/
    char se_text[80]={'\0'};
    snprintf(se_text, 79,"%s(%d)", start_end.name, start_end.on_cycle);
    render_text(renderer, font, x+10, y, se_text);//start_end.name);
    
    int i;
    for(i = 1; i < CITY_NUM; i++) // TODO separate function draw circles
    {
        city rest;
        city_info(&rest, i, READ);
        float x_val = rest.x;
        float y_val = rest.y;
        
        //char name_i[100]={'\0'};
        //sprintf(name_i, "%s %d", rest.name, i);
        //snprintf(name_i,99, "%s %d", rest.name, i);

        //S2D_DrawCircle(x_val, y_val, radius, sectors, 1.0, 0.0, 0.0, 1.0);
        Uint32 color;
        if(rest.on_cycle%2 == 1) // TODO single color
            color=0xFF00FF00;
        else
            color=0xFFFF0000;
        filledCircleColor(*renderer,  x_val,  y_val, 7, color);
        
        //display_text(name_i, x_val, y_val);
        char all_text[80]={'\0'};
        snprintf(all_text, 79,"%s(%d)", rest.name, rest.on_cycle);
        render_text(renderer, font, x_val,  y_val, all_text);//rest.name);//name_i);
    }

    // TODO render joints

    //int nolines = 1;
    //int next = -1;
    //i=0;
    //for(i = 0; nolines < CITY_NUM; nolines++, i = next) 
    /*for(i = 0; next; i = next) 
    {
        city from, to;
        city_info(&from, i, READ);
        next = from.next_i; 
        city_info(&to, next, READ);
        next = to.next_i;
        
        //float width = 2.0;
        //S2D_DrawLine(from.x, from.y, to.x, to.y, width, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2,
          //           1.0, 0.2, 1.0);
        thickLineColor(renderer, from.x, from.y, to.x, to.y, 2, LINE_COLOR);
        printf("i is %d next is %d\n",i,next);
        //if(next==0)
          //  break;
        
    }*/
    city c0,c_1;
    //int prev_i=0;
    city_info(&c0,0,READ);
    int next_i=c0.next_i;
    city from=c0;
    city_info(&c_1,c0.next_i,READ);
    city to=c_1;
    thickLineColor(*renderer, from.x, from.y, to.x, to.y, 2, LINE_COLOR);
    printf("next_i is %d,,,\n",next_i);
    for(int n=0; n< CITY_NUM; n++)
    {
        
        //city_info(&from, prev_i, READ);
        //prev_i=from.next_i;
        //city_info(&to, next_i, READ);//j+1
        //best_path[i] = next; 
        //next = ci.next_i;
        //cj.next_i=best_path[j+1];
        //city_info(&cj, j, WRITE);
        //next_i=to.next_i;
        //city_info(&from, next_i, READ);
        //city_info(&to, from.next_i, READ);// from.next_i n+1
        //printf("next_i is %d from.next_i is %d ",next_i,from.next_i);
        city_info(&from,n , READ);
        city_info(&to, from.next_i, READ);
        thickLineColor(*renderer, from.x, from.y, to.x, to.y, 2, LINE_COLOR);
        next_i=to.next_i;
    }
    printf("\n");
    //print_nexts();
    
    for(int n=0; n<CITY_NUM; n++)
    {
        city_info(&from,n , READ);
        city_info(&to, from.joint, READ);
        if(0)// TODO was: (from.joint != NO_CITY)
            thickLineColor(*renderer, from.x, from.y, to.x, to.y, 1, 0xFFF0FF00);
    }
    printf("\n");

    //city c0, cl;
    //city_info(&c0, 0, READ);
    //city_info(&cl, i, READ);//last_i
    //float width = 2.0;
    //S2D_DrawLine(c0.x, c0.y, cl.x, cl.y, width, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
      //           0.0, 1.0);
    //thickLineColor(renderer,c0.x, c0.y, cl.x, cl.y, 2, LINE_COLOR) ;
    //display_text("t2", 25, 0.85 * Y_MAX); // TODO elapsed time
    
    

    int *v3 = calloc(CITY_NUM + 1, sizeof(int));
    vertex3(v3);

    city c1, c2, c3;
    city_info(&c1, v3[0], READ);
    city_info(&c2, v3[1], READ);
    city_info(&c3, v3[2], READ);
    //S2D_DrawTriangle(c1.x, c1.y, 0, 0, 0, 0.2, c2.x, c2.y, 0, 0, 0, 0.2, c3.x, c3.y, 0, 0, 0, 0.2);
    SDL_Vertex vertices[]=
    {
        { {c1.x, c1.y}, pol, { 0,0 } },// { 255, 0, 0, 255 }
        { {c2.x, c2.y}, pol, { 0,0 } },
        { {c3.x, c3.y}, pol, { 0,0 }}
    };
    SDL_RenderGeometry(*renderer,NULL, vertices,3,NULL,0);
    
    render_sub(renderer, font, time_elapsed);
    
    //show_cycle2(renderer,1); 

    free(v3);
}


// ************************************************ //
void sdl_init(SDL_Window **window,SDL_Renderer **renderer,TTF_Font **font)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf (stderr,"SDL_Init Error: %s", SDL_GetError());
        exit (1);
    }

    *window = SDL_CreateWindow(title, 100, 100, X_MAX, Y_MAX, SDL_WINDOW_OPENGL);
    if (*window == NULL)
    {
        fprintf (stderr,"SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        exit (2);
    }

    *renderer = SDL_CreateRenderer(*window, -1, 0);//SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (*renderer == NULL)
    {
        SDL_DestroyWindow(*window);
        fprintf (stderr,"SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        exit (3);
    }
    int ttfr=TTF_Init();
    if(ttfr!=0)
        exit (4);
    *font = TTF_OpenFont(FONT, POINT_SIZE);
    if (*font==NULL) {
        fprintf(stderr, "The font could not be opened! %s\n", TTF_GetError());
        SDL_Quit();
        exit (5);
    }
}

// ----------------------------------------------------------------------------- //
int main(void)
{
    assert(CITY_NUM >= 3);
    clock_t begin = clock();

#ifdef EXAMPLE_50
    char *names[CITY_NUM+1]={NULL};
    int i=0;
    city cc={.visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
          };
    
    char coord[30];
    FILE * co_50 = fopen("tsp_coord_50.text","rt");
	if(!co_50)
	{
	    fprintf(stderr,"Err opening file.\n");
	    return 1;
	}
	while(1)
	{
		if(feof(co_50))
			break;
		char *p=fgets(coord,30-1,co_50);
        if(p==NULL)
            break;
        sscanf(p,"%f,%f\n",&cc.x,&cc.y);
       
        const int NI_CNT = 20;
        names[i] = malloc(NI_CNT*sizeof(char));
        for(int i2=0; i2<NI_CNT; i2++)
            *(names[i]+i2)='\0';
        sprintf(names[i], "City_%d", i); // TODO remove City_
        cc.name=names[i];
       
        city_info(&cc,i,WRITE);
        i++;
    }
	
	// TODO free names[i]
	fclose(co_50);
#endif
    
    float dis;
    distance_keeper(&dis, READ);
    produce_ways(); 
    
    last_draw();

    int j;
    int *vertices[MAX_CYCLES+1];
    for(j=0; j<MAX_CYCLES+1; j++)
    {
        vertices[j]=calloc(CITY_NUM + 1, sizeof(int)); // TODO init to IMP_COOR_VALUE
        //vertices[j]=malloc( (CITYNUM+1)*sizeof(int));
    }
    int vns[MAX_CYCLES+1]={0}; // ={0};
    for(j=1; j<MAX_CYCLES; j++)// TODO remove debugging code
    {
        //general_cycle(vertices[j],&vns[j],j); 
        
        //bool gcvr=general_cycle_version2(vertices[j],&vns[j],j); 
        bool chr=convex_hull(vertices[j], &vns[j], j);
        if(chr==false)
            break;
        
        
    }
    for(j=0; j<CITY_NUM; j++)
    {
        city cj;
        city_info(&cj, j, READ);
        assert(cj.on_cycle!=NOT_OC);
    }
    print_no_cycle();
    
    for(j=1; j<MAX_CYCLES; j++)
        joints(j);
    for(j=0; j<MAX_CYCLES; j++)
         pre_search(vertices[j],vns[j]);
    
    A_star_algorithm();
    
    // TODO add post_search , eliminate crossed line segments
    
    clock_t end = clock();
    double time_elapsed = (double)(end - begin) / CLOCKS_PER_SEC;
    
    SDL_Window *window=NULL;
    SDL_Renderer *renderer=NULL;
    TTF_Font *font=NULL;
    sdl_init(&window,&renderer,&font);
    SDL_Event event;
    
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(renderer);
    
    render(&renderer, font, time_elapsed); // real job
    
    SDL_RenderPresent(renderer);
        
    while (SDL_WaitEvent(&event))
        if (event.type == SDL_QUIT)
            break;
            
    for(j=0; j<MAX_CYCLES+1; j++)
        free(vertices[j]);
    
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
