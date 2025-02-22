#include "csa.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h> 

#define sort_insert(best, size, a, a_prc, nsize) \
\
{\
unsigned si_i, si_j;\
\
if (size == 0)\
  best[0] = a;\
else\
  {\
  si_j = size;\
  for (si_i = 0; si_i < size; si_i++)\
    if (a_prc < best[si_i]->c - best[si_i]->head->p)\
      {\
      si_j = si_i;\
      for (si_i = nsize - 1; si_i > si_j; si_i--)\
 best[si_i] = best[si_i - 1];\
      break;\
      }\
  best[si_j] = a;\
  }\
}

/* ------------------------- Problem size variables -------------------- */
unsigned	n, m;

/* --------------- Data structures describing the problem -------------- */
lhs_ptr	head_lhs_node, tail_lhs_node;
rhs_ptr	head_rhs_node, tail_rhs_node;
lr_aptr	head_lr_arc, tail_lr_arc;

/* ------------------------- Tunable variables ------------------------- */
/*
 Cost threshhold for pricing out: used even when price-outs are
 switched off to make bounding reduced-cost differences easier in
 double_push(). In principle this is not necessary, but it makes the
 code better.
 */
double		po_cost_thresh;
double		scale_factor;	/* scaling factor */

/*
 Processing variables.
 */
double		epsilon;	/* scaling parameter */
double		min_epsilon;	/* snap to this value when epsilon small */
unsigned	total_e;	/* total excess */
ACTIVE_TYPE	active;		/* list of active nodes */

char *nomem_msg = "Insufficient memory.\n";

void st_reset(csa_stack s) {
    s->top = s->bottom;
}

char *st_pop(csa_stack s) {
    s->top--;
    return(*(s->top));
}

csa_stack st_create(unsigned size) {
    csa_stack s;
    
    s = (csa_stack) malloc(sizeof(struct csa_stack_st));
    if (s == NULL) {
        (void) printf("%s", nomem_msg);
        exit(9);
    }
    s->bottom = (char **) malloc(size * sizeof(char *));
    if (s->bottom == NULL) {
        (void) printf("%s", nomem_msg);
        exit(9);
    }
    s->top = s->bottom;
    return(s);
}

void best_build(lhs_ptr v) {
    unsigned i;
    lr_aptr  a, a_stop;
    double  red_cost, save_max;
    
    for (i = 0, a = v->first; i < NUM_BEST; i++, a++)
    {
        red_cost = a->c - a->head->p;
        sort_insert(v->best, i, a, red_cost, i + 1);
    }
    /*
     Calculate initial next_best by looking at the next arc in the
     adjacency list.
     */
    if ((v->next_best = a->c - a->head->p) <
        (red_cost = v->best[NUM_BEST - 1]->c -
         v->best[NUM_BEST - 1]->head->p))
    {
        sort_insert(v->best, NUM_BEST, a, v->next_best, NUM_BEST);
        v->next_best = red_cost;
    }
    a++;
    /*
     Now go through remaining arcs in adjacency list and place each one
     at the appropriate place in best[], if any.
     */
    a_stop = (v+1)->priced_out;
    for (; a != a_stop; a++)
    {
        if ((red_cost = a->c - a->head->p) < v->next_best)
        {
            if (red_cost < (save_max = v->best[NUM_BEST - 1]->c -
                            v->best[NUM_BEST - 1]->head->p))
            {
                sort_insert(v->best, NUM_BEST, a, red_cost, NUM_BEST);
                v->next_best = save_max;
            }
            else
                v->next_best = red_cost;
        }
    }
}

/* Assume v has excess (is unassigned) and do a double push from v. */
void double_push(lhs_ptr v) {
    double v_pref, v_second, red_cost, adm_gap;
    lr_aptr a, a_stop, adm;
    rhs_ptr w;
    lhs_ptr u;
    unsigned i;
    lr_aptr  *check_arc;
    
    /*
     Begin part I: Compute the following:
     o adm, the minimum-reduced-cost arc incident to v,
     o adm_gap, the amount by which the reduced cost of adm must be
     increased to make it equal in reduced cost to another arc incident
     to v, or enough to price the arc out if it is the only incident
     arc.
     */
    
    if (v->node_info.few_arcs)
    {
        
        /*
         If the input problem is feasible, it is never the case that
         (a_stop == a) after the following two lines because we never get
         excess at a node with no incident arcs.
         */
        a_stop = (v+1)->priced_out;
        a = v->first;
        v_pref = a->c - a->head->p;
        v_second = v_pref + epsilon * (po_cost_thresh + 1.0);
        adm = a;
        /*
         After this loop, v_pref is the minimum reduced cost of an edge out of
         v, and v_second is the second-to-minimum such reduced cost.
         */
        for (a++; a != a_stop; a++)
            if (v_pref > (red_cost = a->c - a->head->p))
            {
                v_second = v_pref;
                v_pref = red_cost;
                adm = a;
            }
            else if (v_second > red_cost)
                v_second = red_cost;
        
    }
    else
    {
        /*
         Find the minimum and second-minimum edges listed in the node's
         best[] array, and check whether their present partial reduced
         costs are below the node's bound as stored in next_best. If they
         are, we calculate adm_gap and are done with part I. If not, we
         rebuild the best[] array and the next_best bound, and calculate the
         required information.
         */
        adm = v->best[1];
        v_second = adm->c - adm->head->p;
        adm = v->best[0];
        v_pref = adm->c - adm->head->p;
        if (v_pref > v_second)
        {
            adm = v->best[1];
            red_cost = v_second;
            v_second = v_pref;
            v_pref = red_cost;
        }
        for (i = NUM_BEST - 2, check_arc = &v->best[2]; i > 0; i--, check_arc++)
        {
            a = *check_arc;
            if (v_pref > (red_cost = a->c - a->head->p))
            {
                v_second = v_pref;
                v_pref = red_cost;
                adm = a;
            }
            else if (v_second > red_cost)
                v_second = red_cost;
        }
        if (v_second > v->next_best)
        {
            /*
             Rebuild the best[] array and recalculate next_best.
             !v->node_info.few_arcs, so we know there are enough incident arcs
             to fill up best[] initially and have one left over for next_best.
             */
            best_build(v);
            adm = v->best[1];
            v_second = adm->c - adm->head->p;
            adm = v->best[0];
            v_pref = adm->c - adm->head->p;
        }
    }
    
    adm_gap = v_second - v_pref;
    
    /*
     Begin part II: Using the information computed in part I,
     o match v to w, adm's head, and
     o unmatch the node (if any) formerly matched to w.
     In the case where w's current matching arc is priced out, we do not
     change the matching, but we reset the value of adm_gap so that the
     (v, w) arc will be priced out.
     */
    w = adm->head;
    if ((u = w->matched))
    /*
     If w's matched arc is priced in, go ahead and unmatch (u, w) and
     match (v, w). If w's matched arc is priced out, abort the double
     push and relabel w so v no longer prefers w.
     */
        if (w->node_info.priced_in)
        {
            u->matched = NULL;
            make_active(u);
            v->matched = adm;
            w->matched = v;
        }
        else
        {
            adm_gap = epsilon * po_cost_thresh;
            make_active(v);
        }
        else
        {
            total_e--;
            v->matched = adm;
            w->matched = v;
        }
    
    /*
     Relabel w: v's price is chosen to make the implicit reduced cost of
     v's new preferred arc (v_pref + adm_gap) equal to zero. Then w's price
     is chosen so that the arc just matched has implicit reduced cost
     -epsilon.
     */
    w->p -= adm_gap + epsilon;
}

void refine() {
    lhs_ptr v;
    
    /*
     Saturate all negative arcs: Negative arcs are exactly those
     right-to-left matching arcs with negative reduced cost, and there is
     an interpretation of the implicit price function on the left that
     admits all right-to-left matching arcs. This interpretation is
     always consistent with the stored prices of lhs nodes in the case
     of EXPLICIT_PRICES.
     */
    total_e = 0;
    for (v = head_lhs_node; v != tail_lhs_node; v++)
    {
        if (v->matched && v->matched->head->node_info.priced_in)
        {
            v->matched->head->matched = NULL;
            v->matched = NULL;
        }
        if (v->matched == NULL)
        {
            total_e++;
            make_active(v);
        }
    }
    
    while (total_e > EXCESS_THRESH)
    {
        get_active_node(v);
        double_push(v);
    }
}

int update_epsilon() {
    int    fix_in = FALSE;
    epsilon /= scale_factor;
    if (epsilon < min_epsilon) {
        epsilon = min_epsilon;
    }
    return(!fix_in);
}

double parse_arcs(csa_arc* arcs, unsigned int arc_count, unsigned int lhs_n, unsigned int rhs_n) {
    unsigned int tail, head, tmp, i;
    long *lhs_degree;
    double cost, max_cost = 0;
    
    lr_aptr a;
    ta_ptr temp_a, temp_arcs;
    lhs_ptr l_v;
    rhs_ptr r_v;
    
    n = lhs_n + rhs_n;
    m = arc_count;

    printf("Parsing %u arcs...\n", arc_count);
    
    head_lr_arc = (lr_aptr) malloc((m + 1) * sizeof(struct lr_arc));
    tail_lr_arc = head_lr_arc + m;
    
    bool swap = false;
    if (lhs_n > n - lhs_n) {
        lhs_n = n - lhs_n;
        swap = true;
    }
    head_lhs_node = (lhs_ptr) malloc((lhs_n + 1) * sizeof(struct lhs_node));
    tail_lhs_node = head_lhs_node + lhs_n;
    head_rhs_node = (rhs_ptr) malloc((n - lhs_n + 1) * sizeof(struct rhs_node));
    tail_rhs_node = head_rhs_node + n - lhs_n;
    lhs_degree = (long *) malloc(lhs_n * sizeof(long));
    
    temp_arcs = (ta_ptr) malloc(m * sizeof(struct temp_arc));
    if ((head_lhs_node == NULL) || (head_lr_arc == NULL) ||
        (lhs_degree == NULL) || (temp_arcs == NULL)) {
        printf("%s", nomem_msg);
        return 0;
    }
    temp_a = temp_arcs;
    for (tail = 0; tail < lhs_n; tail++)
        lhs_degree[tail] = 0;
    
    for(i = 0; i < m; i++) {
    	csa_arc* arc = &arcs[i];
        tail = arc->tail;
        head = arc->head;
        cost = arc->cost;
        
        if (swap) {
        	tmp = head;
        	head = tail;
        	tail = tmp;
        }
        
        if ((tail >= lhs_n) || (head >= n - lhs_n)) {
            printf("Out of range arc: %u=>%u but lhs_n=%u rhs_n=%u\n",
            	arc->head, arc->tail, lhs_n, rhs_n);
            return 0;
        }
        
        temp_a->head = head_rhs_node + head;
        temp_a->tail = head_lhs_node + tail;
        temp_a->cost = cost;
        if (cost > max_cost) {
            max_cost = cost;
        }
        temp_a++;
        lhs_degree[tail]++;
        
    }
    
    a = head_lr_arc;
    for (tail = 0, l_v = head_lhs_node; l_v != tail_lhs_node; l_v++, tail++) {
        l_v->priced_out = l_v->first = a;
        l_v->matched = NULL;
        a += lhs_degree[tail];
        if (lhs_degree[tail] < NUM_BEST + 1) {
            l_v->node_info.few_arcs = true;
        } else {
            l_v->node_info.few_arcs = false;
        }
    }
    tail_lhs_node->priced_out = a;
    
    for (r_v = head_rhs_node; r_v != tail_rhs_node; r_v++) {
        r_v->node_info.priced_in = true;
        r_v->matched = NULL;
        r_v->p = 0.0;
    }
    
    for (temp_a--; temp_a != temp_arcs - 1; temp_a--) {
        a = temp_a->tail->first + (--lhs_degree[temp_a->tail - head_lhs_node]);
        a->head = temp_a->head;
        a->c = (double) temp_a->cost;
    }
    
    (void) free((char *) temp_arcs);
    (void) free((char *) lhs_degree);

    printf("Max cost (epsilon) is %f\n", max_cost);
    
    return max_cost;
}

static double csa(double* cost, unsigned int n, unsigned int* lhs_sol, unsigned int* rhs_sol, double default_scale_factor) {
	// build temporary arcs from the dense cost matrix
    // (ideally we can do this directly to save time, but for now
    // this makes it easier to write the code that does sparse solving)
    unsigned int lhs_n = n;
    unsigned int rhs_n = n;
    unsigned int arc_count = lhs_n * rhs_n;
    printf("Building %u temporary arcs\n", arc_count);
    csa_arc* arcs = malloc(arc_count * sizeof(struct csa_arc_st));
    csa_arc* arcs_itr = arcs;
    double* cost_itr = cost;
    unsigned int tail = 0;
    for(unsigned int row = 0; row < lhs_n; row++) {
        unsigned int head = 0;
        for(unsigned int col = 0; col < rhs_n; col++) {
            csa_arc* arc = arcs_itr++;
            arc->tail = tail;
            arc->head = head;
            arc->cost = *(cost_itr++);
            head++;
        }
        tail++;
    }

    // setup everything for csa from the arcs
    epsilon = parse_arcs(arcs, arc_count, lhs_n, rhs_n);

    // free our temporary arcs
    (void) free((char *) arcs); // should we be freeing with char* or..?

    scale_factor = default_scale_factor;
    po_cost_thresh = 2.0 * (double) n * (scale_factor + 1);
    
    printf("Finding best_build...\n");
    create_active(n);
    lhs_ptr	l_v;
    for (l_v = head_lhs_node; l_v != tail_lhs_node; l_v++)
        if (!l_v->node_info.few_arcs)
            best_build(l_v);
    
    printf("Refining...\n");
    min_epsilon = 2.0 / (double) (n + 1);
    while (epsilon > min_epsilon) {
        (void) update_epsilon();
        printf("\tepsilon: %f min_epsilon: %f\n", epsilon, min_epsilon);
        refine();
    }
    
    printf("Copying solution...\n");
    double cost_sol = 0;
    for (lhs_ptr v = head_lhs_node; v != tail_lhs_node; v++) {
        unsigned int lhs = v - head_lhs_node;
        unsigned int rhs = v->matched->head - head_rhs_node + tail_lhs_node - head_lhs_node - lhs_n;
        double cost = v->matched->c;
        lhs_sol[lhs] = rhs;
        rhs_sol[rhs] = lhs;
        cost_sol += cost;
    }

    return cost_sol;
}