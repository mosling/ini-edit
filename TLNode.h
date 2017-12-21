#include <string>
#include <vector>

using namespace std;

/*!
Treelist node

This class is a simple node for an double connected list with
additional pointer to nodes, which are used to jump over items when a
subtree is not visible.
*/
class TLNode
{
	private:
		string name; //!< name of the variable
		string value; //!< value of the variable
		vector<string> sellist; //!< the selection list
		string help; //!< the help string
		TLNode *succ; //!< the successor node in the list 
		TLNode *prev; //!< the previous node in the list
		TLNode *parent; //!< the parent of an node, if exists
		TLNode *next; //!< the next node with same depth
		TLNode *last; //!< the last node with same depth
		bool hasSelList; //!< true if the entry has an selection list
		bool isOpen; //!< true if the subtree visible
		bool hasSubTree; //!< true if the node has an subtree or is parent
		int depth; //!< depth of the node in the tree, 0 is rootlist

	public:
		//! only a simple constructor
		TLNode () {}; 

		friend class TreeList; //!< class TreeList manage the TLNode connections
};
