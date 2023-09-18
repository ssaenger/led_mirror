/* ********************************************************************************************
 * lists.h
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 12, 2023
 *
 * Description: Basic double linked-list 
 *
 * ********************************************************************************************
 */
#ifndef _LIST_H_
#define _LIST_H_


/* --------------------------------------------------------------------------------------------
 *  TYPES
 * --------------------------------------------------------------------------------------------
 */
typedef struct _ListNode ListNode;

/* --------------------------------------------------------------------------------------------
 * ListNode type
 *
 * Basic double-linked list struct. Both the "list" and "node" use the same structure. The
 * list should be initialized with "InitList()" and the node with "InitNode()"
 *
 */
struct _ListNode {
    struct _ListNode *linkF;
    struct _ListNode *linkB;
};

/* --------------------------------------------------------------------------------------------
 *  MACROS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * InitList macro
 *
 * Initialize the head of a list. Nodes can be placed on the list
 *
 */
#define InitList(list)              (list)->linkF = list;   \
                                    (list)->linkB = list

/* --------------------------------------------------------------------------------------------
 * InitNode macro
 *
 * Initialize a node
 *
 */
#define InitNode(node)              (node)->linkF = 0;      \
                                    (node)->linkB = 0

/* --------------------------------------------------------------------------------------------
 * IsNodeOnList macro
 *
 * Checks if a node is on some list. If either linkF or linkB is non-zero, it can be considered
 * to be on a list.
 *
 */
#define IsNodeUsed(node)           ((node)->linkF != 0)

/* --------------------------------------------------------------------------------------------
 * InsertAfter macro
 *
 * Insert a node after a node on a list
 *
 */
#define InsertAfter(curr, node)     (node)->linkF = (curr)->linkF;  \
                                    (curr)->linkF->linkB = node;    \
                                    (curr)->linkF = node;           \
                                    (node)->linkB = curr

/* --------------------------------------------------------------------------------------------
 * InsertTail macro
 *
 * Insert a node at the end of the list
 *
 */
#define InsertBefore(curr, node)    (node)->linkB = (curr)->linkB;  \
                                    (node)->linkB->linkF = node;    \
                                    (node)->linkF = curr;           \
                                    (curr)->linkB = node

/* --------------------------------------------------------------------------------------------
 * InsertHead macro
 *
 * Insert a node at the head of the list.
 *
 */
#define InsertHead(list, node)      InsertAfter(list, node)

/* --------------------------------------------------------------------------------------------
 * InsertTail macro
 *
 * Insert a node at the end of the list
 *
 */
#define InsertTail(list, node)      InsertBefore(list, node)

/* --------------------------------------------------------------------------------------------
 * RemoveNode macro
 *
 * Remove a node from a list
 *
 */
#define RemoveNode(node)            (node)->linkB->linkF = (node)->linkF;  \
                                    (node)->linkF->linkB = (node)->linkB;  \
                                    InitNode(node)

/* --------------------------------------------------------------------------------------------
 * GetNextNode macro
 *
 * Get the next node on the list. Don't remove it.
 *
 */
#define GetNextNode(node)           (node)->linkF

/* --------------------------------------------------------------------------------------------
 * GetPriorNode macro
 *
 * Get the prior node on the list. Don't remove it.
 *
 */
#define GetPriorNode(node)           (node)->linkB

/* --------------------------------------------------------------------------------------------
 * GetHead macro
 *
 * Get the first node of a list. Don't remove it.
 *
 */
#define GetHead(list)               GetNextNode(list)

/* --------------------------------------------------------------------------------------------
 * IsListEmpty macro
 *
 * Checks if the list is empty or not
 *
 */
#define IsListEmpty(list)           ((list)->linkF == (list))

/* --------------------------------------------------------------------------------------------
 * IterateList macro
 *
 * A for loop to iterate through all the nodes of the list. The nodes cannot be removed
 * while iterating through the list. Use brackets {} at the end of this macro and the
 * contents inside the brackets is what will be executed in each iteration through the list.
 *
 */
#define IterateList(list, curr, type)  for ((curr) = (type) GetHead(&list) ;        \
                                            (curr) != (type)&(list);                \
                                            (curr) = (type)GetNextNode(&(curr)->node))

/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */
int IsNodeOnList(ListNode *List, ListNode *Node);

#endif /* _LIST_H_ */
   