#include "ForwardIndex.h"
#include "Commmon.h"


ForwardIndex::ForwardIndex(void)
{

}


ForwardIndex::~ForwardIndex(void)
{
}

int ForwardIndex::load_index()
{

	return 0;
}


/*
**Êý¾Ý  id  path          
**×Ö½Ú:  4  len+path   
** 
**/
int ForwardIndex::save_index()
{
	FILE* file=fopen("","wb");
	if ( file!=NULL )
	{
		std::list<Document*>::iterator iter=m_documents.begin();
		for ( ; iter!=m_documents.end() ; ++iter )
		{
			iter->doc_id  
		}
		
		fclose(file);
	}

	return 0;
}

int ForwardIndex::add_doc( Document* doc )
{
	m_documents.push_back(doc);
	return 0;
}
