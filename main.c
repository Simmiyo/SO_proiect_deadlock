#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<error.h>
#include <sys/types.h>
#include<errno.h>
#include<time.h>
#include <stdbool.h>

pthread_mutex_t mtxA,mtxB,mtxGate,mtxSeg,mtxLock;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int seg_counter = 0, lock_counter = 0, m_counter = 0;
bool go=false;

//Diana
struct segmentation_context_nod
{
	pthread_t tid;
    int segment;
	struct segmentation_context_nod *nxt;
};

struct segment_node_adjlist
{
	int seg;
	struct segment_node_adjlist *next;
};
struct segment_adjlist
{
	struct segment_node_adjlist *head;
};

struct segment_adjlist GS[50];
struct segmentation_context_nod *CS=NULL;
//Diana


//Simona
struct lock_segment
{
    int l;
    int s;
};

struct lock_context_nod
{
	pthread_t tid;
	struct lock_segment lk_sg[15];
	int N;
	struct lock_context_nod *nxt;
};

struct muchie
{
    int l1;
    int l2;
    pthread_t t;
    int s1;
    int s2;
    int lks[15];
    int n;
};

struct lock_node_adjlist
{
	int lk;
	struct lock_node_adjlist *next;
};
struct lock_adjlist
{
	struct lock_node_adjlist *head;
};

struct lock_adjlist GL[15];
struct lock_context_nod *CL=NULL;
struct muchie muchii[60];
//Simona

//Diana
void new_segment(int x)
{
    struct segment_node_adjlist *q = (struct segment_node_adjlist *)malloc(sizeof(struct segment_node_adjlist ));
    q->seg = x;
    q->next = NULL;
    GS[x].head = q;
}

void init_CS()
{
    CS = (struct segmentation_context_nod *)malloc(sizeof(struct segmentation_context_nod));
    CS->tid = 0;
    CS->segment = seg_counter;
    CS->nxt = NULL;
}

void start(pthread_t id1, pthread_t id2)
{
    pthread_mutex_lock(&mtxSeg);
    struct segmentation_context_nod *p = CS;
    while(p!=NULL && !pthread_equal(p->tid,id1))
            p = p->nxt;
    if(pthread_equal(p->tid,id1))
    {
        //Update GS
        new_segment(p->segment);
        struct segment_node_adjlist *q1 = (struct segment_node_adjlist *)malloc(sizeof(struct segment_node_adjlist ));
        q1->seg = seg_counter+1;
        struct segment_node_adjlist *q2 = (struct segment_node_adjlist *)malloc(sizeof(struct segment_node_adjlist ));
        q2->seg = seg_counter+2;
        q2->next = NULL;
        q1->next = q2;
        GS[p->segment].head->next = q1;

        //Update CS
        p->segment = seg_counter + 1;

        struct segmentation_context_nod *p1 = CS;
        while(p1->nxt!=NULL)
            p1 = p1->nxt;

        struct segmentation_context_nod *p2 = (struct segmentation_context_nod *)malloc(sizeof(struct segmentation_context_nod ));
        p2->tid = id2;
        p2->segment = seg_counter + 2;
        p1->nxt = p2;
        p2->nxt = NULL;

        seg_counter += 2;
        go = false;
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mtxSeg);
}

void join(pthread_t id1, pthread_t id2)
{
    struct segmentation_context_nod *p = CS;
    while(p!=NULL && !pthread_equal(p->tid,id1))
            p = p->nxt;
    if(pthread_equal(p->tid,id1))
    {
        //Update GS
        new_segment(p->segment);
        struct segment_node_adjlist *q1 = (struct segment_node_adjlist *)malloc(sizeof(struct segment_node_adjlist ));
        q1->seg = seg_counter+1;
        q1->next = NULL;
        GS[p->segment].head->next = q1;

        //Update CS
        p->segment = seg_counter + 1;
    }
    p = CS;
    while(p!=NULL && !pthread_equal(p->tid,id2))
            p = p->nxt;
     if(pthread_equal(p->tid,id2))
     {
        //Update GS
        new_segment(p->segment);
        struct segment_node_adjlist *q2 = (struct segment_node_adjlist *)malloc(sizeof(struct segment_node_adjlist ));
        q2->seg = seg_counter+1;
        q2->next = NULL;
        GS[p->segment].head->next = q2;
     }
     seg_counter += 1;
}
//Diana

//Simona
void new_lock()
{
    struct lock_node_adjlist *q = (struct lock_node_adjlist *)malloc(sizeof(struct lock_node_adjlist ));
    q->lk = lock_counter;
    q->next = NULL;
    GL[lock_counter].head = q;
    lock_counter++;
}

int search_seg(pthread_t x)
{
    pthread_mutex_lock(&mtxSeg);
    while(go)
         pthread_cond_wait(&cond,&mtxSeg);

    struct segmentation_context_nod *s=CS;
    while(s!=NULL && !pthread_equal(s->tid,x))
            s = s->nxt;
    pthread_mutex_unlock(&mtxSeg);
    return s->segment;

}

void new_muchie(int l1,int s1,pthread_t id,struct lock_segment ls[],int n,int l2,int s2)
{
    int i;
    muchii[m_counter].l1 = l1;
    muchii[m_counter].l2 = l2;
    muchii[m_counter].t = id;
    muchii[m_counter].s1 = s1;
    muchii[m_counter].s2 = s2;
    for(i=0;i<n;i++)
        muchii[m_counter].lks[i] = ls[i].l;
    muchii[m_counter].n = n;
    for(i=n;i<15;i++)
         muchii[m_counter].lks[i] = -1;
    m_counter++;
}

void adauga_GL(struct lock_node_adjlist *h,int l)
{
	while(h->next != NULL && h->lk!=l)
		h = h->next;
	if(h->lk!=l)
    {
        struct lock_node_adjlist *nou;
        nou =(struct lock_node_adjlist *)malloc(sizeof(struct lock_node_adjlist ));
        nou->lk = l;
        nou->next = NULL;
        h->next = nou;
    }
}

void aquire(pthread_t id,int lock)
{
    pthread_mutex_lock(&mtxLock);
    int segment = search_seg(id);
	if(CL==NULL)
	{
		CL = (struct lock_context_nod *)malloc(sizeof(struct lock_context_nod));
		CL->tid = id;
		CL->lk_sg[0].l = lock;
		CL->lk_sg[0].s = segment;
		CL->N=1;
		CL->nxt = NULL;
	}
	else
	{
		int nr=0;
		struct lock_context_nod *p=CL,*pred=NULL;
		while(p != NULL)
		{
			if(pthread_equal(p->tid,id))
				break;
			pred = p;
			p = p->nxt;
		}
		if(p!=NULL)
		{
			while(nr < p->N)
			{
                struct lock_node_adjlist *inceput = GL[p->lk_sg[nr].l].head;
                adauga_GL(inceput,lock);
                new_muchie(p->lk_sg[nr].l, p->lk_sg[nr].s, id, p->lk_sg, p->N, lock, segment);
				nr++;
			}
			p->lk_sg[p->N].l = lock;
			p->lk_sg[p->N].s = segment;
			p->N++;
		}
		else
        {
            p = (struct lock_context_nod *)malloc(sizeof(struct lock_context_nod));
            p->tid = id;
            p->lk_sg[0].l = lock;
            p->lk_sg[0].s = segment;
            p->N=1;
            p->nxt = NULL;
            pred->nxt = p;
        }
	}
    pthread_mutex_unlock(&mtxLock);
}

void release(pthread_t id,int lock)
{
    struct lock_context_nod *p=CL;
    while(p != NULL)
    {
			if(pthread_equal(p->tid,id))
				break;
			p = p->nxt;
    }
    if(p != NULL)
    {
        int i=0;
        for(i=0;i<p->N;i++)
        {
            if(p->lk_sg[i].l == lock)
            {
                p->lk_sg[i].l = -1;
                p->lk_sg[i].s = -1;
                p->N--;
                break;
            }
        }
    }
}
//Simona

//Cristi
bool same_thread(int i,int j)
{
    if(pthread_equal(muchii[i].t,muchii[j].t))
        return true;
    return false;
}
bool lock_overlap(int k,int p)
{
    int j,i;
    for(i=0;i<muchii[k].n;i++)
    {
        for(j=0;j<muchii[p].n;j++)
        {
            if(muchii[k].lks[i]==muchii[p].lks[j])
                return true;
        }
    }
    return false;
}
bool check_path(int s,int d,bool vizitat[])
{
    if(s==d)
        return true;
    else
    {
        vizitat[s] = true;
        struct segment_node_adjlist *p = GS[s].head;
        while(p!=NULL)
        {
            p = p->next;
            if(p!=NULL && !vizitat[p->seg] && check_path(p->seg,d,vizitat))
                return true;
        }
        return false;
    }
}
bool ordered_segments(int k,int p)
{
    int e1,e2,i;
    e1 = muchii[k].s2;
    e2 = muchii[p].s1;
    bool vizitat[seg_counter];
    for(i=0;i<seg_counter;i++)
        vizitat[i] = false;
    return check_path(e1,e2,vizitat);
}

bool false_pozitive(bool muchii_stack[],int m)
{
    int i;bool ok = false;
    for(i=0;i<m_counter;i++)
    {
        if(muchii_stack[i] == true)
        {
            ok = true;
            if(same_thread(i,m) || lock_overlap(i,m) || ordered_segments(i,m))
                return true;
        }
    }
    return false;
}

int visit_muchie(int v1,int v2,bool m_visited[],bool m_stack[])
{
    int i;
    for(i=0;i<m_counter;i++)
    {
        if(muchii[i].l1 == v1 && muchii[i].l2 == v2 && !m_visited[i] && !false_pozitive(m_stack,i))
            return i;
    }
    return -1;
}

void unvisit_muchii(int v1,int v2,bool m_visited[],bool m_stack[])
{
    int i;
    for(i=0;i<m_counter;i++)
    {
        if(muchii[i].l1 == v1 && muchii[i].l2 == v2 && !m_visited[i])
        {
            m_visited[i] = true;
            m_stack[i] = true;
        }
    }
}

bool DFS_cycle(int v,bool visited[],bool stack[],bool m_visited[],bool m_stack[])
{
    if(visited[v]==false)
    {
        int m;
        visited[v] = true;
        stack[v] = true;
        struct lock_node_adjlist *p = GL[v].head->next;
        while(p!=NULL)
        {
                while((m = visit_muchie(v,p->lk,m_visited,m_stack))>-1)
                {
                    m_visited[m] = true;
                    m_stack[m] = true;
                    if(!visited[p->lk] && DFS_cycle(p->lk,visited,stack,m_visited,m_stack))
                        return true;
                    else if(stack[p->lk])
                            return true;
                        else
                            m_stack[m] = false;
                }
                p = p->next;
        }
    }
    stack[v] = false;
    visited[v] = false;
    return false;
}

int check_cycle()
{
    int i;
    bool visited[lock_counter],stack[lock_counter],m_visited[m_counter],m_stak[m_counter];
    for(i=0;i<lock_counter;i++)
    {
         visited[i] = false;
         stack[i] = false;
    }
    for(i=0;i<m_counter;i++)
    {
        m_stak[i] = false;
        m_visited[i] = false;
    }


    for(i=0;i<2;i++)
        if(DFS_cycle(i,visited,stack,m_visited,m_stak))
            return 1;
    return 0;
}
//Cristi

//Simona
void *thr3(void *null)
{
    sleep(1);
    pthread_t pt=pthread_self();
    aquire(pt,0);
	pthread_mutex_lock(&mtxA);
	aquire(pt,1);
	pthread_mutex_lock(&mtxB);

	pthread_mutex_unlock(&mtxB);
	release(pt,1);
	pthread_mutex_unlock(&mtxA);
	release(pt,0);
}

void *thr1(void *null)
{
    pthread_t pt=pthread_self();

    aquire(pt,2);
	pthread_mutex_lock(&mtxGate);
	aquire(pt,0);
	pthread_mutex_lock(&mtxA);
	aquire(pt,1);
	pthread_mutex_lock(&mtxB);

	pthread_mutex_unlock(&mtxB);
	release(pt,1);
	pthread_mutex_unlock(&mtxA);
	release(pt,0);
	pthread_mutex_unlock(&mtxGate);
	release(pt,2);

	pthread_t t3;
	go=true;
	if(pthread_create(&t3,NULL,thr3,NULL))
	{
		perror("thread 3");
		return NULL;
	}
	start(pt,t3);

	pthread_join(t3,NULL);
	join(pt,t3);

    aquire(pt,1);
	pthread_mutex_lock(&mtxB);
	aquire(pt,0);
	pthread_mutex_lock(&mtxA);

	pthread_mutex_unlock(&mtxA);
	release(pt,0);
	pthread_mutex_unlock(&mtxB);
	release(pt,1);
}

void *thr2(void *null)
{
    pthread_t pt=pthread_self();
    aquire(pt,2);
	pthread_mutex_lock(&mtxGate);
	aquire(pt,1);
	pthread_mutex_lock(&mtxB);
	aquire(pt,0);
	pthread_mutex_lock(&mtxA);

	pthread_mutex_unlock(&mtxA);
	release(pt,0);
	pthread_mutex_unlock(&mtxB);
	release(pt,1);
	pthread_mutex_unlock(&mtxGate);
	release(pt,2);
}

int main(int argc, char *argv[])
{
    init_CS();
	if(pthread_mutex_init(&mtxA,NULL))
	{
		perror("mutex A");
		return errno;
	}
	new_lock();
	if(pthread_mutex_init(&mtxB,NULL))
	{
		perror("mutex B");
		return errno;
	}
	new_lock();
	if(pthread_mutex_init(&mtxGate,NULL))
	{
		perror("mutex Gate");
		return errno;
	}
    new_lock();
    if(pthread_mutex_init(&mtxSeg,NULL))
	{
		perror("mutex Seg");
		return errno;
	}
	if(pthread_mutex_init(&mtxLock,NULL))
	{
		perror("mutex Lock");
		return errno;
	}

	pthread_t t1,t2;
	go=true;
	if(pthread_create(&t1,NULL,thr1,NULL))
	{
		perror("thread 1");
		return errno;
	}
	start(0,t1);
	go=true;
	if(pthread_create(&t2,NULL,thr2,NULL))
	{
		perror("thread 2");
		return errno;
	}
	start(0,t2);

	pthread_join(t1,NULL);
	join(0,t1);
	pthread_join(t2,NULL);
	join(0,t2);

	int c = check_cycle();
	if(c)
        printf("DEADLOCK!");
    else
        printf("NO DEADLOCK!");
	pthread_mutex_destroy(&mtxA);
	pthread_mutex_destroy(&mtxB);
	pthread_mutex_destroy(&mtxGate);
	pthread_mutex_destroy(&mtxSeg);
	pthread_mutex_destroy(&mtxLock);
	return 0;
}
//Simona
