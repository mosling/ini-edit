#include "TLNode.h"

/*!
class TreeList

This class implements the double connected list, using class TLNode as
list nodes. Methods for setting and getting data and an iterator which
browse through the list building the interface.
*/
class TreeList
{
	private:
		TLNode *root; //!< pointer to the root list-node
		TLNode *last; //!< pointer to the last item for faster insert
		TLNode *iterator; //!< pointer to the active list-node
		void CheckIterator (); //!< class invariant
		TLNode *PrivateAddTreePointer (TLNode *p, TLNode *node, int d);
		TLNode *PrivateSuccIterator ();
		bool isChanged; //!< true if something was changed
		bool mergeEqual; //!< if true equal variable names are merged
		int Member (const char *);

	public:
		//! the constructor set the pointers to 0 and nothing is changed
		TreeList () 
		{
			root = 0;
			iterator = 0;
			last = 0;
			isChanged = false;
			mergeEqual = true;
		}
		void AddEntry (string, string, string, const vector<string>&, int);
		void AddTreePointer ();
		void TreeOpen (bool);
		const char *GetVar ();
		const char *GetVal ();
		const char *GetHelp ();
		//! returns the number of possible selections
		int NumSelections () { return iterator->sellist.size(); }
		//! returns the vector of selections
		vector<string>& SelectionList () {return iterator->sellist;}
		void SetVal (string );
		void SetVal (unsigned int);
		//! returns the depth of a node that is a value greater-equal 0
		int GetDepth () {return iterator->depth;}
		void Show ();
		bool Search (const char *); 
		//! return true if v an member of the selection list
		bool IsMember (const char *v) { return Member(v)!=-1;}
		bool SuccIterator (); 
		bool PrevIterator ();
		bool SetIterator (int);
		//! set the iterator of the variable with name 
		bool SetIterator (const char *name) { return Search (name)==true;};
		//! returns true, if the variable has an selectionlist
		bool HasSelection () {return iterator->hasSelList; }
		void SetNextSelection ();
		//! returns true if the list is empty
		bool Empty () { return (root==0);}
		//! returns true if the iterator stand on the last list-item
		bool Last () { return (PrivateSuccIterator()==0x0);}
		//! return true if the iterator stands outside the list
		bool End () { return (iterator == 0); }
		//! set the iterator to the first list-item
		bool First () { return (iterator == root); }
		//! return true if something is changed
		bool IsChanged() { return isChanged; }
		//! set the isChanged variable to false, after save for example
		void NotChanged() { isChanged = false; }
		//! return true if the node has a subtree
		bool HasSubTree() { return iterator->hasSubTree; }
		//! open (b=true) or close (b=false) the subtree
		void IsOpen (bool b) { iterator->isOpen = b; }
		//! return true if the subtree is visible (or open)
		bool IsOpen () { return iterator->isOpen; }
		//! set the mergeEqual variable to b
		void MergeEqual (bool b) { mergeEqual = b; }
		void OpenParents ();
		void WriteIniFile (const char *, bool);
};
