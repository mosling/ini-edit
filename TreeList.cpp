#include <iostream>
#include <fstream>
#include "TreeList.h"

/*!
\brief class invariante

this functions checks the validity of the iterator. If the value is
zero the program is exited.
This furious exit is not very intelligent, but this error should only
occur in the development phase.
*/
void TreeList::CheckIterator ()
{
	if (iterator == 0) 
	{
		cerr << "iterator is 0 (class invariant failed)" << endl;
		exit (1);
	}
}

/*!
\brief show the contents of the list

This functions is only for development, for writing Ini-Files use
WriteIniFile. We need our own iterator, because the SuccIterator and
PrevIterator-functions jump over closed subtrees.
*/
void TreeList::Show ()
{
	TLNode *it = root;
	unsigned int k;
	string null("NULL");

	while ( it != 0x0 )
	{
		cout << "--------------------------------------" << endl;
		cout << "Name : /" << it->name << "/" << endl;
		cout << "Wert : /" << it->value << "/" << endl;
		cout << "Hilfe: /" << it->help << "/" << endl;
		if (it->sellist.size() > 0)
		{
			cout << "Selektionsliste ("<< it->hasSelList << "):" << endl;
			for (k=0; k<it->sellist.size(); k++)
				cout << k << ".Element: /" << it->sellist[k] << "/" << endl;
		}
		cout << "Pointer:" << endl;
		cout << "succ  :" << (it->succ!=0x0?it->succ->name:null) << endl;
		cout << "prev  :" << (it->prev!=0x0?it->prev->name:null) << endl;
		cout << "next  :" << (it->next!=0x0?it->next->name:null) << endl;
		cout << "last  :" << (it->last!=0x0?it->last->name:null) << endl;
		cout << "parent:" << (it->parent!=0x0?it->parent->name:null) << endl;
		cout << "HasSubTree: " << it->hasSubTree << endl;
		cout << "IsOpen    : " << it->isOpen << endl;
		it = it->succ;
	}
}

/*!
\brief writing the ini file

This writes the list as Ini-File, if complete is false only the
(variable,value) pair is written, else the helptext including the
selection list and categories is written.
We need our own iterator, because the SuccIterator and
PrevIterator-functions jump over closed subtrees.
*/
void TreeList::WriteIniFile (const char *fname, bool complete)
{
	ofstream out(fname);
	TLNode *lit, *it = root;
	unsigned int k;
	string tstr;

	if (!out.is_open())
	{
		cerr << "can't open file " << fname << endl;
		return;
	}

	if (complete) out << "#-created by IniEdit" << endl;
	while ( it != 0x0 )
	{
		if (complete)
		{
			//! first check the ending of an subtree
			//! this funny if construction uses no lazy evaluation
			if (it->prev != 0x0)
				if (it->depth < it->prev->depth)
			{
				//! There are two cases
				//! case1: it->depth is zero
				if (it->depth == 0)
					out << endl << "#C finish";
				//! case2: more than one step --> lot of '#C end's
				else
				{
					int i;

					for (i=it->prev->depth; i>it->depth; i--)
						out << endl << "#C end";
				}
			}
			
			tstr = it->help;
			if (tstr.size() > 0)
			{
				// replace all \n with "\n# "
				for (k=0; k<tstr.size(); k++)
				{
					if (tstr[k] == '\n')
						tstr.replace (k, 1, "\n# ");
				}
				out << endl << "# " << tstr;
			}
		}
		if (it->hasSubTree)
		{
			if (complete) out << endl << "#C " << it->name << endl;
		}
		else
		{
			out << endl << it->name << " = " << it->value << endl;
		}
		lit = it;
		it = it->succ;
	}
	out.close();
}

/*!
\brief search for an entry with name vname

This searches the list for an entry and set the iterator to it, if it
found, else iterator is unchanged and the function returns false.
\bug (not really)
At this time only exact matches with an variable name are found. For
regular expression search I will add the methods NextSearch and
PrevSearch for jumping between the matches.
*/
bool TreeList::Search (const char *vname)
{
	TLNode *tmp = root;
	string str(vname);

	while ( tmp != 0 )
	{
		if (str == tmp->name)
		{
			iterator = tmp;
			return true;
		}
		tmp = tmp->succ;
	}
	return false;
}

/*!
Search for an given value v in the selectionlist, returns the position
or -1 if not found
*/
int TreeList::Member (const char *v)
{
	unsigned int k;

	for (k=0; k<iterator->sellist.size(); k++)
	{
		if (iterator->sellist[k] == v)
			return k;
	}
	return -1;
}

/*!
\brief add an entry with given name, value, helptext and selection vector

If the variable var exists the new value is set, the selection 
and the help-text is append to the existing variable. Else a new
variable is created. The internal pointers are initailized. The
hasSubTree variable is set to false and the isOpen variable is set to
true, so you can use the TreeList for representing simple lists,
without an knowing tree-structure. Without this initialization the
methods SuccIterator and PrevIterator don't work right.
*/
void TreeList::AddEntry (string var, string val, 
	string hlp, const vector<string>& vstr, int d)
{
	TLNode *it = iterator;
	unsigned int k;

	//! remove the trailing \n
	if (hlp.size() > 0)
	{
		k = hlp.size()-1;
		while ((k>=0) && (hlp[k] == '\n'))
			k--;
		hlp.erase (k+1);
	}

	//! look for an long variable-name
	if (var.length() > 25)
	{
		//! cut it
		if (hlp.length() == 0) { hlp = var; }
		var.erase(25);
		var = var + "...";
	}

	if (Search (var.c_str()) && mergeEqual==true) 
	{
		//! variable exists
		if (hlp.size() > 0)
		{
			iterator->help.append ("\n");
			iterator->help.append (hlp);
		}
		if (vstr.size() > 0)
		{
			for (k=0; k<vstr.size(); k++)
				iterator->sellist.push_back (vstr[k]);
		}
		iterator->value = val;
	}
	else
	{	
		//! eine neue Variable erzeugen
		TLNode *tmp = new TLNode;
		tmp->prev = last;
		tmp->succ = 0;
		if (last != 0) last->succ = tmp;
		last = tmp;
		if (root == 0) root = tmp;
		tmp->name = var;
		tmp->value = val;
		tmp->help = hlp;
		tmp->isOpen = true;
		tmp->hasSubTree = false;
		tmp->depth = d; //! depth is set by new variables only
		if (vstr.size() > 0)
		{
			tmp->sellist = vstr;
		}

		iterator = tmp;
	}
	//! if the selection list has greater size than 1 an selection list exists
	if (iterator->sellist.size() > 1)
		iterator->hasSelList = true;
	else
		iterator->hasSelList = false;

	//! if exists an selection list, the value must be a member of the list
	if (iterator->hasSelList == true)
	{
		if (!IsMember (val.c_str()))
		{
			SetVal (0);
		}
	}

	iterator = it;
}

/*!
\brief The interface function for PrivateAddTreePointer.
*/
void TreeList::AddTreePointer ()
{
	if (root->depth != 0)
	{
		cerr << "first list-item-depth must be zero (0)" << endl;
		exit (1);
	}
	PrivateAddTreePointer ((TLNode *)0x0, root, 0);
}

/*!
\brief add the tree pointers

This functions add pointers to the list node, so that SuccIterator and
PrevIterator can jump over closed subtrees. The following pictures
shows the used pointers.
\verbatim
      +------+
      |node 1|
      +------+
        |  ^
	succ |  | prev
        v  |
      +------+
      |node 2|<-----+
      +------+----+ |
      | ^ ^       | |
		| | |       | |
      | | parent  v |
      | | |  +------+
      n l +- |node 3|
      e a |  +------+
		x s |       | ^
      t t parent  v |
      | | |  +------+
      | | +- |node 4|
      | |    +------+
      v |         | ^
      +------+<---+ |   
      |node 5|------+
      +------+
\endverbatim
An node with an subtree is identified if the depth is less than the
depth of the next node. In this case PrivateAddTreePointer is called
recursively. The parameter p is the parent of the parameter node and d
is the actual depth of node. Start is with p=0x0 and node=root, the
first depth is 0.
*/
TLNode *TreeList::PrivateAddTreePointer (TLNode *p, TLNode *node, int d)
{
	TLNode *tmpnode;

	if (p != (TLNode *)0x0)
	{
		// all subtrees are open
		p->isOpen = true;
		p->hasSubTree = true;
	}

	while (node != 0x0)
	{
		if (node->depth == d)
		{
			node->next = 0x0;
			node->last = node->prev;
			node->parent = p;
		}
		else if (node->depth > d)
		{
			tmpnode = node->prev;
			node = PrivateAddTreePointer (node->prev, node, d+1);
			tmpnode->next = node;
			if (node == 0x0) return node;
			if (node->depth == d)
			{
				node->last = tmpnode;
				node->parent = p;
			}
			else
			{
				return node;
			}
		}
		else // node->depth < d
			return node;

		node = node->succ;
	}
	return (TLNode*)0x0;
}

/*!
\brief open or close all subtrees  

The node property isOpen is set to %b.
*/
void TreeList::TreeOpen (bool b)
{
	TLNode *it = root;

	while ( it != 0x0)
	{
		if (it->hasSubTree) it->isOpen = b;
		it = it->succ;
	}
}

/*!
\brief returns a pointer to the name of the variable
*/
const char *TreeList::GetVar ()
{
	CheckIterator();
	return iterator->name.c_str();
}

/*! 
\brief returns a pointer to the value of the variable 
*/
const char *TreeList::GetVal ()
{
	CheckIterator();
	return iterator->value.c_str();
}

/*! 
\brief returns a pointer to the helptext of the variable 
*/
const char *TreeList::GetHelp ()
{
	CheckIterator();
	return iterator->help.c_str();
}

/*!
\brief Set the value of the variable to val.
*/
void TreeList::SetVal (string val)
{
	CheckIterator();
	if (!isChanged)
	{
		if (iterator->value != val)
			isChanged = true;
	}
	iterator->value = val;
}

/*!
Set the value of the variable to s'th entry of the selection list
if the entry exists.
\pre s must be between 0 and number of selections
*/
void TreeList::SetVal (unsigned int s)
{
	if (s>=0 && s<iterator->sellist.size())
	{
		if (!isChanged)
		{
			if (iterator->value != iterator->sellist[s])
				isChanged = true;
		}
		iterator->value = iterator->sellist[s];
	}
}

/*!
\brief The value is cyclic rotated trougth the selection list.
*/
void TreeList::SetNextSelection ()
{
	int item,s;

	if (iterator->hasSelList)
	{
		item = Member (iterator->value.c_str());
		s = iterator->sellist.size();
		SetVal ( (item+1)%s);
	}
}

/*!
This function is used for the Last-method. We look one step ahead and
see a zero-pointer or an regular node. If we have an zero-pointer the
Last method know that the actual node is the last possible node.
*/
TLNode *TreeList::PrivateSuccIterator ()
{
	TLNode *tmpit;

	if (iterator != 0)
	{
		if (iterator->isOpen)
			tmpit = iterator->succ;
		else
			tmpit = iterator->next;
	}

	return tmpit;
}

/*!
\brief set the iterator to the next list-item

The iterator is set to the next node, following the pointer succ for
normal nodes. The pointer next is used for closed subtrees. After the
last node the iterator is set to 0x0. 
*/
bool TreeList::SuccIterator ()
{
	iterator = PrivateSuccIterator ();
	return (iterator != 0x0); 
}

/*!
\brief Set the iterator of its previous list-item

This is a little bit complexer than SuccIterator, because we have the
choice between more than two values. We search the last item from an
open subtree before iterator. We need a while-loop, because the node
which is pointered by iterator->prev can be closed and not visible.
*/
bool TreeList::PrevIterator ()
{
	TLNode *tmpit, *tmpp;

	if (iterator != 0)
	{
		if ((iterator->last != iterator->prev) && iterator->last->isOpen)
		{
			tmpit = iterator->prev;
			tmpp = iterator->prev;
			while (tmpit->parent != iterator->last)
			{
				tmpit = tmpit->prev;

				if (tmpit->hasSubTree && !tmpit->isOpen)
					tmpp = tmpit;
			}
			iterator = tmpp;
		}
		else
			iterator = iterator->last;
		return true;
	}
	return false;
}

/*!
\brief Set the iterator to the pos'th list-item.

If pos greater than the number of nodes the method return false and
the iterator is set to zero (0x0);
*/
bool TreeList::SetIterator (int pos)
{
	int i=0;

	iterator = root;
	while (i<pos)
	{
		if (SuccIterator() == false)
		{
			return false;
		}
		i++;
	}
	return true;
}

/*!
Open all parents, so the variable is visible. This method is call
after a search from an view-interface.
*/
void TreeList::OpenParents ()
{
	TLNode *it = iterator;

	while (it->parent != 0x0)
	{
		it = it->parent;
		it->isOpen = true;
	}
}
