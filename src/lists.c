/* ********************************************************************************************
 * lists.c
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 14, 2023
 *
 * Description: Source file for the lists.h
 *
 * ********************************************************************************************
 */
#include "../inc/lists.h"

/* --------------------------------------------------------------------------------------------
 *                 IsNodeOnList()
 * --------------------------------------------------------------------------------------------
 * Description:    Returns the location of the selected list where the node is, or 0 if it is
 *                 not on the list
 *
 * Parameters:     List - The list to search for the node
 *                 Node - The node to look for in the list
 *
 * Returns:        0 if not on the list or a non-zero value to indicate location in the list
 */
int IsNodeOnList(ListNode *List, ListNode *Node)
{
    ListNode *nodeItr;
    int       loc = 0;
    if (!IsNodeUsed(Node)) {
        return 0;
    }
    nodeItr = List;
    while ((nodeItr = GetNextNode(nodeItr)) != List) {
        loc++;
        if (Node == nodeItr) {
            break;
        }
    }
    return loc;
}