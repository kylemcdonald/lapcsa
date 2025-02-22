#define    TRUE    1
#define    FALSE    0
#define    DEFAULT_SCALE_FACTOR    10
#define    EXCESS_THRESH    0
#define    ACTIVE_TYPE    csa_stack
#define    create_active(size)    active = st_create(size)
#define    make_active(v)        st_push(active, (char *) v)
#define    get_active_node(v)    v = (lhs_ptr) st_pop(active)

#define    st_push(s, el) \
{\
*(s->top) = (char *) el;\
s->top++;\
}

#define    st_empty(s)    (s->top == s->bottom)

#define    enq(q, el) \
{\
*(q->tail) = el;\
if (q->tail == q->end) q->tail = q->storage;\
else q->tail++;\
}

#define q_empty(q) (q->head == q->tail ? 1 : 0)

#define    insert_list(node, head) \
{\
node->next = (*(head));\
(*(head))->prev = node;\
(*(head)) = node;\
node->prev = tail_rhs_node;\
}

#define    delete_list(node, head) \
{\
if (node->prev == tail_rhs_node) (*(head)) = node->next;\
node->prev->next = node->next;\
node->next->prev = node->prev;\
}

#define	NUM_BEST	3

typedef	struct	lhs_node	{
    struct	{
        /*
         flag used to indicate to
         double_push() that so few arcs
         are incident that best[] is
         useless.
         */
        unsigned	few_arcs : 1;
    }	node_info;
    /*
     list of arcs to consider first in
     calculating the minimum-reduced-cost
     incident arc; if we find it here, we
     need look no further.
     */
    struct	lr_arc	*best[NUM_BEST];
    /*
     bound on the reduced cost of an arc we
     can be certain still belongs among
     those in best[].
     */
    double	next_best;
    
    /*
     first arc in the arc array associated
     with this node.
     */
    struct	lr_arc	*priced_out;
    /*
     first priced-in arc in the arc array
     associated with this node.
     */
    struct	lr_arc	*first;
    /*
     matching arc (if any) associated with
     this node; NULL if this node is
     unmatched.
     */
    struct	lr_arc	*matched;
}	*lhs_ptr;

typedef	struct	rhs_node	{
    struct	{
        /*
         flag to indicate this node's
         matching arc (if any) is
         priced in.
         */
        unsigned	priced_in : 1;
    }	node_info;
    /*
     lhs node this rhs node is matched to.
     */
    lhs_ptr	matched;
    /*
     price of this node.
     */
    double	p;
}	*rhs_ptr;

typedef	struct	lr_arc		{
    /*
     rhs node associated with this arc.
     */
    rhs_ptr	head;
    /*
     arc cost.
     */
    double	c;
}	*lr_aptr;

typedef	struct	csa_stack_st	{
    /*
     Sometimes csa_stacks have lhs nodes, and
     other times they have rhs nodes. So
     there's a little type clash;
     everything gets cast to (char *) so we
     can use the same structure for both.
     */
    char	**bottom;
    char	**top;
}	*csa_stack;

typedef	struct	csa_queue_st	{
    /*
     Sometimes csa_queues have lhs nodes, and
     other times they have rhs nodes. So
     there's a little type clash;
     everything gets cast to (char *) so we
     can use the same structure for both.
     */
    char		**head;
    char		**tail;
    char		**storage;
    char		**end;
    unsigned	max_size;
}	*csa_queue;

typedef	struct	temp_arc	{
    lhs_ptr	tail;
    rhs_ptr	head;
    double	cost; // seems strange this is long, shouldn't it be double?
}	*ta_ptr;

typedef struct csa_arc_st {
	/*
	We define our own arc format for ease of use.
	*/
	unsigned int tail;
	unsigned int head;
	double cost; 
} csa_arc;