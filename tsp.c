// Originally from:
// https://github.com/DubiousCactus/GeneticAlgorithm

//#define NDEBUG
//#define EXAMPLE_8
#define EXAMPLE_50

#include <assert.h>
#include <math.h>
#include <simple2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef EXAMPLE_8
#define xMax 800 
#define yMax 600 
#define CITY_NUM 8 // TODO compute it , many random cities.
#endif

#ifdef EXAMPLE_50
#define xMax 1000 
#define yMax 1000
#define CITY_NUM 50
#endif

#define ALL_VISITED -1
#define NOT_OC 0 //-1
#define LAST_LAYER -1
//#define MAX_EDGES CITY_NUM
#define MAX_CYCLES 10
#define NO_CITY -1

static int last_i = 0;

enum RW
{
    READ = 0,
    WRITE
}; // TODO add FREE
const int sectors = 500;
GLfloat radius = 5.0;
struct city
{
    char *name;
    GLfloat x;
    GLfloat y;
    // visited
    bool visited;
    int on_cycle;
    // struct city *next;
    int joint; // to an outer? (inner) cycle
    int next_i;
};
typedef struct city city;

// ************************************************ //

void display_text(char *in_text, GLfloat x, GLfloat y);

bool correct_index(int i)
{
    if(i == ALL_VISITED || (i >= 0 && i < CITY_NUM))
        return true;

    return false;
}

void distance_keeper(GLfloat *dis, enum RW rw)
{
    static GLfloat kdis = 0;

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
        
        {.x = xMax * 0.47,
         .y = yMax * 0.03,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Lille"}, // Beginning city
        {.x = xMax * 0.51,
         .y = yMax * 0.15,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Paris"},
        {.x = xMax * 0.8,
         .y = yMax * 0.06,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Reims"},
        {.x = xMax * 0.7,
         .y = yMax * 0.65,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Lyon"},
        {.x = xMax * 0.9,
         .y = yMax * 0.7,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Marseille"},
        {.x = xMax * 0.15,
         .y = yMax * 0.3,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "Nantes"},
        {.x = xMax * 0.05,
         .y = yMax * 0.42,
         .visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
         .name = "La Rochelle"},
        {.x = xMax * 0.2,
         .y = yMax * 0.59,
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

GLfloat distance(city from, city to)
{
    GLfloat dis = 0.0;
    GLfloat dx = fabsf(to.x - from.x);
    GLfloat dy = fabsf(to.y - from.y);

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
    assert(no_cycles < MAX_CYCLES);
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
    GLfloat dis = xMax + yMax;
    city out_c;

    for(i = 0; i < CITY_NUM; i++) 
    {
        city_info(&out_c, i, READ);
        if(out_c.visited == true || i == since)
        {
            continue;
        }

        GLfloat curr_dis = distance(out_c_s, out_c);
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

        GLfloat dis = distance(cn, cn2);
        distance_keeper(&dis, WRITE);

        display_text("t1 inside loop", 25, 0.8 * yMax);
        
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

    GLfloat pre_dis;
    distance_keeper(&pre_dis, READ);
    GLfloat dis = distance(ci, cn);
    distance_keeper(&dis, WRITE);

    last_i = i;
    return;
}

// ************************************************ //

int vertex(GLfloat x, GLfloat y)
{
    int i;
    int cnt = 0;
    for(i = 0; i < CITY_NUM; i++)
    {
        city ci;
        city_info(&ci, i, READ);
        if(ci.on_cycle == NOT_OC)
            cnt++;
    }
    if(cnt < 3)
        return LAST_LAYER; 

    city eg = {.x = x, .y = y};
    int save_i = 0;
    GLfloat min_dis = xMax + yMax;
    city ci; 
    for(i = 0; i < CITY_NUM; i++)
    {
        city_info(&ci, i, READ);
        GLfloat dis = distance(ci, eg);
        if(dis < min_dis && ci.on_cycle == NOT_OC) 
        {
            min_dis = dis;
            save_i = i;
        }
    }

    assert(correct_index(save_i));
    return save_i;
}

bool vertex3(int *v3)
{
    
    v3[0] = vertex(xMax / 2, 0);
    v3[1] = vertex(0, yMax);
    v3[2] = vertex(xMax, yMax);

    if(v3[0] == LAST_LAYER || v3[1] == LAST_LAYER || v3[2] == LAST_LAYER)
        return false;
    return true;
}

GLfloat min(GLfloat a, GLfloat b)
{
    GLfloat min;
    min = (a < b) ? a : b;
    return min;
}
GLfloat max(GLfloat a, GLfloat b)
{
    GLfloat max;
    max = (a > b) ? a : b;
    return max;
}

#define MIN_DIFF 5
bool lines_cross(city c1, city c2, city c3, city c4)
{
    //assert(c1.x != c3.x || c1.y != c3.y); // TODO more like this  ?
    
    
    assert(c2.x != c1.x);
    GLfloat a = (c2.y - c1.y) / (c2.x - c1.x);
    GLfloat b = c1.y - a * c1.x;

    assert(c4.x != c3.x);
    GLfloat c = (c4.y - c3.y) / (c4.x - c3.x);
    GLfloat d = c3.y - c * c3.x;

    GLfloat x, y;
    
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
        x > (min(c1.x, c2.x)) && x < (max(c1.x, c2.x)) && x > (min(c3.x, c4.x)) && x < (max(c3.x, c4.x)) &&
        y > (min(c1.y, c2.y)) && y < (max(c1.y, c2.y)) && y > (min(c3.y, c4.y)) && y < (max(c3.y, c4.y)))
        return true;
    return false;
}

bool inside_cycle(int *vertices, int vertices_num, int io_ind)
{
    
    assert(vertices_num >= 3);
    
    int i;
    
    city in_out;
    city_info(&in_out, io_ind, READ);// TODO compute and find these
    city ci;
    
    int v3[3];
    bool er = vertex3(v3);
    if(er == false)
        return true;
    printf("v1 is %d v2 is %d v3 is %d \n", v3[0], v3[1], v3[2]);

    for(i = 0; i < CITY_NUM; i++) 
    {
        if(i == io_ind)
            continue; 
        if(!(i == v3[0] || i == v3[1] || i == v3[2]))
            continue; 
        if((io_ind == v3[0] || io_ind == v3[1] || io_ind == v3[2]))
            continue; 
        city_info(&ci, i, READ);

        bool cond = false;
        int j = 0;
        int k = 0;
        int l = 0;
        for(l = 0; l < vertices_num; l++)
        {
            for(j = 0; j < vertices_num; j++)
            {
                if(l == j)
                    continue;

                city cl, cj;
                city_info(&cl, vertices[l], READ);
                city_info(&cj, vertices[j], READ);
                for(k = 0; k < vertices_num; k++)
                {
                    if(k == io_ind)
                        continue;
                    
                    city ck;
                    city_info(&ck, vertices[k], READ);
                    if(lines_cross(ck, in_out, cl, cj) &&
                       (l == j + 1 || j == l + 1 || (j == 0 && l == vertices_num - 1) ||
                        (l == 0 && j == vertices_num - 1))) 
                    {
                        cond = true;
                    }
                }
            }
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
    }

    return true;
}
bool outside_cycle(int *vertices, int vertices_num, int io_ind)
{
    bool r;
    
    int i;
    for(i = 0; i < vertices_num; i++)
    {
        city ci;
        city_info(&ci, i, READ);

        if(io_ind == vertices[i])
            return false;
    }
    r = !inside_cycle(vertices, vertices_num, io_ind);
    return r;
}

void print_vertices(int *vertices, int vertices_num)
{
    int i;
    for(i = 0; i < vertices_num; i++)
        printf("vertices[%d] is %d ", i, vertices[i]);
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

void expand_cycle(int *vertices, int vertices_num, int out_ind)
{
    int i, j;
    int e1 = 0, e2 = 0;
    GLfloat dis1 = xMax + yMax;
    GLfloat dis2 = xMax + yMax;
    city out_city;
    city_info(&out_city, out_ind, READ);
    assert(!inside_cycle(vertices, vertices_num, out_ind));
    for(i = 0; i < vertices_num; i++)
        assert(out_ind != vertices[i]);

    print_vertices(vertices, vertices_num);
    for(i = 0; i < vertices_num - 1; i++)
        for(j = i + 1; j < vertices_num; j++)
        {
            if(i == j)
                continue;

            city ci;
            city_info(&ci, vertices[i], READ);
            city cj;
            city_info(&cj, vertices[j], READ);

            if(distance(ci, out_city) < dis1 && !lines_cross(ci, out_city, ci, cj) &&
               vertices[i] != e2) // TODO no cross
            {
                dis1 = distance(ci, out_city);
                e1 = vertices[i];
            }

            if(distance(cj, out_city) < dis2 && !lines_cross(cj, out_city, ci, cj) && vertices[j] != e1) 
            {
                dis2 = distance(cj, out_city);
                
                e2 = vertices[j];
            }
        }
    if(e1 == e2)
        printf("e1 is %d e2 is %d out_ind is %d \n", e1, e2, out_ind);
    assert(e1 != e2);

    if(e2 == vertices[vertices_num - 1] || e1 == vertices[vertices_num - 1])
    {
        vertices[vertices_num] = out_ind;
        print_vertices(vertices, vertices_num + 1);
        return;
    }
    for(i = vertices_num - 1; i > 0; i--)
    {
        vertices[i + 1] = vertices[i];
        if(vertices[i] == e2 || vertices[i] == e1) 
        {
            vertices[i] = out_ind;
            break;
        }
    }

    print_vertices(vertices, vertices_num + 1);
}

// TODO expand cycle 3 to more.
bool general_cycle(int *vertices, int *vertices_num, int cycle) 
{
    // TODO inside vertices
    *vertices_num = 0;
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
                cycle_keeper(&i, i, cycle, WRITE); 
                vertices[*vertices_num] = i;
                (*vertices_num)++;
            }
        }
        print_vertices(vertices, *vertices_num);
        return true; // LAST_LAYER;
    }

    for(i = 0; i < CITY_NUM; i++) 
    {
        
        city in_out;
        city_info(&in_out, i, READ);
        bool op;
        op = outside_cycle(vertices, cnt, i); 
        if(op)
        {
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

    *vertices_num = cnt;
    return false; 
}
//////////////////////////////////////
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
void joints(int cycle) 
{
    assert(cycle >= NOT_OC);
    assert(cycle <= MAX_CYCLES);
    int i;
    int j_cnt = 0;
    int oj_cnt = 0;
    city ci;
    int *in_joint = calloc(CITY_NUM, sizeof(int)); 
    int *out_joint = calloc(CITY_NUM, sizeof(int)); 

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
    //assert(oj_cnt >= 3); // TODO for now removed
    printf("cycle is %d oj_cnt is %d j_cnt is %d\n",cycle,oj_cnt,j_cnt);
    if(oj_cnt<=2)
    {
        
        return;
    }

    if(j_cnt == 0)
        return;
    if(j_cnt == 1) // TODO debug
    {
        city cji, cjo;
        city_info(&cji, in_joint[0], READ);

        city_info(&cjo, out_joint[0], READ);
        GLfloat dis1 = distance(cji, cjo);
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
        GLfloat dis2 = distance(cji, cjo);
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
        GLfloat dis1 = distance(cji, cjo); // TODO dis=MAX_DIST
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

        GLfloat dis2 = distance(cji, cjo);
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

    for(j = 0; j < CITY_NUM; j++)
    {
        city_info(&cj, j, READ);

        int i1 = (((j + 1) >= vcnt) ? 0 : (j + 1));
        int i2 = (((j - 1) < 0) ? vcnt - 1 : (j - 1));
        
        if(ci.on_cycle == cj.on_cycle &&
           i == vertices[j]) 
        {
            if(next_node[2] == NO_CITY) // TODO gen this pattern
                next_node[2] = vertices[i1]; 
        }
        else if(ci.on_cycle == cj.on_cycle && i == vertices[i1]) 
        {
            if(next_node[3] == NO_CITY)
                next_node[3] = vertices[j];
        }
        if(ci.on_cycle == cj.on_cycle && i == vertices[j]) // TODO rotation ?
        {
            if(next_node[4] == NO_CITY)
                next_node[4] = vertices[i2];
        }
        else if(ci.on_cycle == cj.on_cycle && i == vertices[i2])
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
            printf("%d,", nk[j]);
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
#define MAX_STATES 50 // 1000 25 15 10000
int g_vertex[CITY_NUM * MAX_NEXT]; // TODO remove global array
struct st_t
{
    //int vertex[CITY_NUM * MAX_NEXT]; // TODO maybe extra , only once
    int path[CITY_NUM+1];
    int stack[CITY_NUM+1];
    // uint64_t hash; // TODO sum of all city but NO_CITY
    int step; // TODO steps in path
    GLfloat dist;
};
typedef struct st_t st_t;

void print_path(int *path)
{
    printf("......\n");
    int i;
    for(i = 0; i < CITY_NUM + 1; i++)
        path[i]==NO_CITY ? printf("- "):printf("%d ", path[i]);
    printf("\n");    
}

void print_state(st_t *states, int si)
{
    int i;
    printf("======\n");
    printf("si is %d , dist is %.1lf step is %d\n"
        , si, (states + si)->dist,(states + si)->step);
    
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
    
    printf("......\n");
    
    printf("path : ");
    for(i = 0; i < CITY_NUM + 1; i++)
        (states+si)->path[i]==NO_CITY ? printf("- "):printf("%d ", (states+si)->path[i]);
    printf("\n");   
    
    printf("stack: ");
    for(i = 0; i < CITY_NUM + 1; i++)
        (states+si)->stack[i]==NO_CITY ? printf("- "):printf("%d ", (states+si)->stack[i]);
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
void states_keeper(st_t *states, int si, enum RW rw) // TODO path
{
    int ci;
    
    assert(si >= 0);
    assert(si < MAX_STATES);
    if(rw == READ)
    {
        for(ci = 0; ci < CITY_NUM; ci++)
            nexts_keeper(&g_vertex[ci * MAX_NEXT], ci, READ);
        path_keeper((states+si)->path, READ);
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
        int pk[CITY_NUM+1];
        path_keeper(pk,READ);
        
        for(int j=0; j<CITY_NUM+1; j++)
            (states + si)->path[j]=pk[j];
        path_keeper((states+si)->path, WRITE);
    }
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
        printf("%d : nc[0] is %d nc[1] is %d sc is %d,%d,%d,%d nc[5] is %d\n", i, nexts[0], nexts[1], nexts[2],
               nexts[3], nexts[4], nexts[5], nexts[6]);
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
            printf("%d,", nk[j]);
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
void match(st_t *states,int *si_ptr);

void remove_city(int city)//, st_t *state)
{
    int l;
    
    for( l=0; l<CITY_NUM*MAX_NEXT; l++)
    {
        if(g_vertex[l]==city )
        {
            g_vertex[l]=NO_CITY;
        }
    }
    
}

void eliminate(st_t *state)
{
    int j,l;
    
    for( j=1; (j<CITY_NUM+1) && state->path[j]!=NO_CITY; j++)
        for( l=0; l<CITY_NUM*MAX_NEXT; l++)
        {
            if(g_vertex[l]==state->path[j] )
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
        print_state(st1_ptr,0);
        print_state(st2_ptr,0);
        exit(1);
    }
    
    // TODO dist
    return true;
}
bool equal_path(st_t* state1, st_t* state2)
{
    for(int i=0; i<CITY_NUM+1; i++)
        if(state1->path[i]!=state2->path[i])
            return false;
    return true;
}

bool on_path(int city, st_t * state)
{
    int pi;
    for(pi=0; pi<CITY_NUM+1; pi++)
    {
        if(city==state->path[pi])
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

bool possible_next(int next_city, st_t* state)
{
    
    print_state(state,0);
    
    if(next_city==0 && state->path[CITY_NUM-1]!=NO_CITY)
    {
        return true;
    }
    
    
    int j;
    
    for(j=0; j<MAX_NEXT; j++)
    {
        int index=next_city*MAX_NEXT+j;
        int city=g_vertex[index];
        if(city!=NO_CITY && !on_path(city,state))
            return true;
    }
    return false;
}    
bool impossible(st_t *state)
{
    if(all_nocity())
        return true;
        
    int i;
    for(i=0; i<CITY_NUM; i++)
        if(possible_next(i,state))
            return false;
    
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
bool complement(st_t *state) // TODO use it 
{
    
    printf(" #$#$#$ \n");
    print_state(state,0);
    
    
    int city;
    
    for(city=0; city<CITY_NUM; city++)
    {
        if(on_path(city, state))
            continue;
        if(on_state(city))
            continue;
        return false;
    }
    return true;
}

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
        print_state(states,i);
        //assert(complement(states+i));
        if(equal_path(states+si,states+i) && !all_nocity()) // TODO not executed
        {
            *(states+*si_ptr+1)=*(states+i); // TODO overwrite , hold all states in memory
            print_state(states,i);
            
            break;
        }
    }
    if(i==0)
    {
        fprintf(stderr, " _+_+_+_+_ \n");
        print_state(states,i);
        print_state(&save_state,0);
        print_state(states,si);
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
}
// ....................................... //
// https://www.programiz.com/dsa/graph-dfs
void push(int ci, st_t *state)
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
}
bool path_is_full(st_t *state) // TODO distance , etc
{
    int i;
    for(i=0; i<=CITY_NUM; i++)
        if(state->path[i]==NO_CITY)
            return false;
    return true;
}
//    ======================================    //
void dfs_algorithm(void) // Depth first search
{
    int i,max_i=0;
    
    remove_repetition();
    print_keeper();

    st_t *states = calloc(MAX_STATES+1, sizeof(st_t));
    
    GLfloat min_dist;
    distance_keeper(&min_dist, READ);
    int path0[CITY_NUM+1]={NO_CITY};
    path0[0]=0;
    
    for(i=1; i<CITY_NUM+1; i++)
        path0[i]=NO_CITY;
    path0[CITY_NUM]=0; // recently added
    path_keeper(path0, WRITE);
    path_keeper(path0,READ);
    print_path(path0);

    printf("++++++++++++++++++++++++++++++++++++++++\n");
    printf("min_dist is %lf\n", min_dist);

    states_keeper(states, 0, WRITE);
    
    for(i=0; i<CITY_NUM+1; i++)
        states[0].stack[i]=NO_CITY;
    for(i=0; i<CITY_NUM+1; i++)
    {
        if(g_vertex[i]==NO_CITY)
        {   
            break;
        }
        states[0].stack[i]=g_vertex[i];
        //states[0].stack[i]=NO_CITY; // init stack
    }
    
    
    states_keeper(states, 1, READ);
    print_state(states, 0);
    print_state(states, 1);
    
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

    // TODO total distance(later min dis) is a cap
    // TODO stop on distance cap or any remaining city with no choice
    for(i = 1; i < MAX_STATES; i++) // TODO success forward backward
    {
        //if(stack_is_empty(&states[i]))
        //    break;
        states[i] = states[i - 1];
        
        
        assert(complement(states+i));
        if(i>max_i)
            max_i=i;
        
        print_state(states, i);
        
        /*if(i>=3 && equal(states+i,states+i-2))// TODO idle
            for(int j = CITY_NUM ; j >0 ; j--)
            {
                int cpj=(states+i)->path[j];
                if(cpj!=NO_CITY)
                {
                    (states+i)->path[j]=NO_CITY;
                    
                    match(states,&i);
                    
                    break;
                }
            }*/
        assert(complement(states+i));
        
        /*if(impossible(states+i))
        { 
            step_backward(states, &i);   
        }
        else{ // TODO keep this or not ?
        
        bool better_found = step_forward(states, i, &min_dist);
        if(better_found)
            for(int j = 0; j < CITY_NUM + 1; j++)
                best_path[j] = (states+i)->path[j];
        }
        
        assert(complement(states+i));*/
        
        int top=pop(&states[i]);
        int j;
        for(j=0; j<=CITY_NUM+1; j++) // j=0
        {
            if((states+i)->path[j]==NO_CITY)
            {
                (states+i)->path[j]=top;
                city c1,c2;
                city_info(&c1, top, READ);
                city_info(&c2, (states+i)->path[j-1], READ);
                (states+i)->dist += distance(c1,c2);
                break;
            }
            
        }
        // TODO step 3:
        //int j;
        for(j = 0; j < MAX_NEXT; j++)
        {
            int ci=g_vertex[top * MAX_NEXT + j];
                
            bool op=on_path( ci,states+i);
            if( ci != NO_CITY && !op)
            {
                push(ci,&states[i]);
            }
                
        }
        if(path_is_full(&states[i]) && states[i].dist<min_dist)
        {
            min_dist=states[i].dist;
            for( j = 0; j <= CITY_NUM + 1; j++)
                best_path[j] = (states+i)->path[j];
        }
                
        if(stack_is_empty(&states[i]))
        {
            print_state(states,i);
            break;
        }    
    }
    printf("min_dist is %.1lf max_i is %d\n", min_dist,max_i);
    print_path(best_path);

    free(states);
    return;
}

// ************************************************ //
// TODO draw cycle
void show_cycle2(int on_cycle)
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

            S2D_DrawTriangle(c1.x, c1.y, 0, 0, 0, 0.1, c2.x, c2.y, 0, 0, 0, 0.1, c3.x, c3.y, 0, 0, 0, 0.1);
            break;
        }
    }
}

void show_cycle(int on_cycle) 
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

            S2D_DrawTriangle(c1.x, c1.y, 0, 0, 0, 0.1, c2.x, c2.y, 0, 0, 0, 0.1, c3.x, c3.y, 0, 0, 0, 0.1);
            
        }
    }
}

void display_text(char *in_text, GLfloat x, GLfloat y)
{
    
    assert(strlen(in_text) <= 60);
    S2D_Text *text = S2D_CreateText("/usr/share/fonts/gnu-free/FreeSans.ttf", in_text, 15);
    text->x = x + 10; // 50
    text->y = y; 
    text->color.r = 0.0;
    text->color.g = 0.0;
    text->color.b = 0.0;
    text->color.a = 1.0;
    S2D_DrawText(text);
    S2D_FreeText(text);
}

void render(void) // TODO show elapsed
{
    
    city start_end;
    city_info(&start_end, 0, READ); // begin from here
    GLfloat x = start_end.x;
    GLfloat y = start_end.y;
    GLfloat dis0 = 0;
    distance_keeper(&dis0, WRITE);

    S2D_DrawCircle(x, y, radius, sectors, 0.0, 0.0, 1.0, 1.0);

    S2D_Text *beg_text = S2D_CreateText("/usr/share/fonts/gnu-free/FreeSans.ttf", start_end.name, 15); // TODO common font
    beg_text->x = x + 10;
    beg_text->y = y;
    beg_text->color.r = 0.1;
    beg_text->color.g = 0.1;
    beg_text->color.b = 0.1;
    beg_text->color.a = 1.0;
    S2D_DrawText(beg_text);
    S2D_FreeText(beg_text);

    int i;
    for(i = 1; i < CITY_NUM; i++) // TODO separate function draw circles
    {
        city rest;
        city_info(&rest, i, READ);
        GLfloat x_val = rest.x;
        GLfloat y_val = rest.y;
        
        char name_i[100];
        sprintf(name_i, "%s %d", rest.name, i);

        S2D_DrawCircle(x_val, y_val, radius, sectors, 1.0, 0.0, 0.0, 1.0);
        
        display_text(name_i, x_val, y_val);
    }

    int nolines = 1;
    int next = 0;
    for(i = 0; nolines < CITY_NUM; nolines++, i = next) 
    {
        city from, to;
        city_info(&from, i, READ);
        next = from.next_i; 
        city_info(&to, next, READ);
        
        GLfloat width = 2.0;
        S2D_DrawLine(from.x, from.y, to.x, to.y, width, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2, 1.0, 0.2,
                     1.0, 0.2, 1.0);
        
    }

    city c0, cl;
    city_info(&c0, 0, READ);
    city_info(&cl, last_i, READ);
    GLfloat width = 2.0;
    S2D_DrawLine(c0.x, c0.y, cl.x, cl.y, width, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
                 0.0, 1.0);

    //display_text("t2", 25, 0.85 * yMax); // TODO elapsed time
    
    char ta[80];
    GLfloat dis;
    distance_keeper(&dis, READ);
    sprintf(ta, "Total distance is %.1lf cities_count is %d", dis, CITY_NUM);
    display_text(ta, 50, 0.9 * yMax);

    int *v3 = calloc(CITY_NUM + 1, sizeof(int));
    vertex3(v3);

    city c1, c2, c3;
    city_info(&c1, v3[0], READ);
    city_info(&c2, v3[1], READ);
    city_info(&c3, v3[2], READ);
    S2D_DrawTriangle(c1.x, c1.y, 0, 0, 0, 0.2, c2.x, c2.y, 0, 0, 0, 0.2, c3.x, c3.y, 0, 0, 0, 0.2);
    
    show_cycle2(1); 

    free(v3);
}

void update(void)
{
}

// ************************************************ //
int main(void)
{
    assert(CITY_NUM >= 3);

#ifdef EXAMPLE_50
    char *names[CITY_NUM+1]={NULL};
    int i=0;
    city cc={.visited = false,
         .on_cycle = NOT_OC,
         .joint = NO_CITY,
         .next_i = ALL_VISITED,
          };
    //char name[10];
    char coord[30];
    FILE * co_50 = fopen("tsp_coord_50.text","rt");
	if(!co_50)
	{
	    fprintf(stderr,"Err  opening file.\n");
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
       
       //char name[ENOUGH];//calloc
       names[i] = malloc(20*sizeof(char));
       sprintf(names[i], "City_%d", i);
       cc.name=names[i];
       
       city_info(&cc,i,WRITE);
       i++;
    }
	
	fclose(co_50);
#endif

    const S2D_Color color = {0.9, 0.9, 0.9, 1.0};
    char *title = "TSP problem , graph theory";
    
    GLfloat dis;
    distance_keeper(&dis, READ);
    produce_ways(); 
    
    last_draw();

    /*
    int *vertices = calloc(CITY_NUM + 1, sizeof(int)); // TODO static inside a function
    int *vertices2 = calloc(CITY_NUM + 1, sizeof(int));
    
    int vn1, vn2;
    general_cycle(vertices, &vn1, 1); // TODO inside a loop
    general_cycle(vertices2, &vn2, 2); 
    
    print_no_cycle();
    
    joints(2);

    pre_search(vertices, vn1);
    pre_search(vertices2, vn2);*/
    int j;
    int *vertices[MAX_CYCLES+1];
    for(j=0; j<MAX_CYCLES; j++)
        vertices[j]=calloc(CITY_NUM + 1, sizeof(int));
    int vns[MAX_CYCLES+1];
    for(j=0; j<MAX_CYCLES; j++)
        general_cycle(vertices[j],&vns[j],j+1);
    print_no_cycle();
    //for(j=MAX_CYCLES; j>1; j--)
    for(j=1; j<MAX_CYCLES; j++)
        joints(j);
    for(j=0; j<MAX_CYCLES; j++)
         pre_search(vertices[j],vns[j]);
    
    dfs_algorithm();

    S2D_Window *window = S2D_CreateWindow(title, xMax, yMax, NULL, render, 0 // TODO remove update: NULL
    );
    window->background = color;
    window->frames = 1;

    S2D_Show(window);

    S2D_FreeWindow(window);
    
    //free(vertices);
    //free(vertices2);
    for(j=0; j<MAX_CYCLES; j++)
        free(vertices[j]);
    
    return 0;
}
