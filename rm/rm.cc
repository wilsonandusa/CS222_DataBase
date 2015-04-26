
#include "rm.h"
#define DEBUG 1

RC PrepareCatalogDescriptor(string tablename,vector<Attribute> &attributes){
	string tables="Tables";
	string columns="Columns";
	Attribute attr;

	if(tables.compare(tablename)==0){
		attr.name="table-id";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=1;
		attributes.push_back(attr);

		attr.name="table-name";
		attr.type=TypeVarChar;
		attr.length=50;
		attr.position=2;
		attributes.push_back(attr);

		attr.name="file-name";
		attr.type=TypeVarChar;
		attr.length=50;
		attr.position=3;
		attributes.push_back(attr);

		attr.name="SystemTable";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=4;
		attributes.push_back(attr);


		return 0;
	}
	else if(columns.compare(tablename)==0){
		attr.name="table-id";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=1;
		attributes.push_back(attr);

		attr.name="column-name";
		attr.type=TypeVarChar;
		attr.length=50;
		attr.position=2;
		attributes.push_back(attr);

		attr.name="column-type";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=3;
		attributes.push_back(attr);

		attr.name="column-length";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=4;
		attributes.push_back(attr);

		attr.name="column-position";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=5;
		attributes.push_back(attr);

		attr.name="NullFlag";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=6;
		attributes.push_back(attr);


		return 0;
	}
	else{
		cout<<"Error! PrepareCatalogDescriptor can only be used to get Catalog's record descriptor" <<endl;
		return -1;
	}

}

RC CreateTablesRecord(void *data,int tableid,string tablename,int systemtable){
	int offset=0;
	int size=tablename.size();
	char nullind=0;

	//copy null indicator
	memcpy((char *)data+offset,&nullind,1);
	offset=offset+1;

	memcpy((char *)data+offset,&tableid,sizeof(int));
	offset=offset+sizeof(int);
	//copy table name
	memcpy((char *)data+offset,&size,sizeof(int));
	offset=offset+sizeof(int);

	memcpy((char *)data+offset,tablename.c_str(),size);
	offset=offset+size;

	//copy file name
	memcpy((char *)data+offset,&size,sizeof(int));
	offset=offset+sizeof(int);

	memcpy((char *)data+offset,tablename.c_str(),size);
	offset=offset+size;

	//copyt SystemTable
	memcpy((char *)data+offset,&systemtable,sizeof(int));
	offset=offset+sizeof(int);
	#ifdef DEBUG
		cout<<endl<<"create table record "<<"offset is "<<offset<<endl;
	#endif
	return 0;

}

RC CreateColumnsRecord(void * data,int tableid, Attribute attr, int position, int nullflag){
	int offset=0;
	int size=attr.name.size();
	char null[1];
	null[0]=0;


	//null indicator
	memcpy((char *)data+offset,null,1);
	offset+=1;

	memcpy((char *)data+offset,&tableid,sizeof(int));
	offset=offset+sizeof(int);

	//copy VarChar
	memcpy((char *)data+offset,&size,sizeof(int));
	offset=offset+sizeof(int);
	memcpy((char *)data+offset,attr.name.c_str(),size);
	offset=offset+size;

	//copy  type
	memcpy((char *)data+offset,&(attr.type),sizeof(int));
	offset=offset+sizeof(int);

	//copy attribute length
	memcpy((char *)data+offset,&(attr.length),sizeof(int));
	offset=offset+sizeof(int);

	//copy position
	memcpy((char *)data+offset,&position,sizeof(int));
	offset=offset+sizeof(int);

	//copy nullflag
	memcpy((char *)data+offset,&nullflag,sizeof(int));
	offset=offset+sizeof(int);
	#ifdef DEBUG
		cout<<endl<<"create column record "<<"offset is "<<offset<<endl;
	#endif
	return 0;

}
RC UpdateColumns(int tableid,vector<Attribute> attributes){
	int size=attributes.size();
	RecordBasedFileManager *rbfm=RecordBasedFileManager::instance();
	FileHandle table_filehandle;
	char *data=(char *)malloc(PAGE_SIZE);
	vector<Attribute> columndescriptor;
	RID rid;

	PrepareCatalogDescriptor("Columns",columndescriptor);
	if(rbfm->openFile("Columns", table_filehandle)==0){
		for(int i=0;i<size;i++){
			CreateColumnsRecord(data,tableid,attributes[i],i+1,0);
			rbfm->insertRecord(table_filehandle,columndescriptor,data,rid);
		}
		rbfm->closeFile(table_filehandle);
		free(data);
		return 0;
	}
	#ifdef DEBUG
		cout<<"There is bug on UpdateColumns"<<endl;
	#endif
	return -1;
}

int GetFreeTableid(){

	RM_ScanIterator rm_ScanIterator;
	RID rid;
	char *data=(char *)malloc(PAGE_SIZE);

	vector<string> attrname;
	attrname.push_back("table-id");
	int tableID;
	int foundID;
	bool scanID[TABLE_SIZE];
	std::fill_n(scanID,TABLE_SIZE,0);


	if(RelationManager::scan("Tables","",NO_OP,NULL,attrname,rm_ScanIterator)==0){
		while(rm_ScanIterator.getNextTuple(rid,data)!=RM_EOF){
			//!!!! skip null indicator
			memcpy(&foundID,(char *)data+1,sizeof(int));
			scanID[foundID-1]=true;

		}
		for(int i=0;i<TABLE_SIZE;i++){
			if(!scanID[i]){
				tableID=i+1;
				break;
			}
		}

		free(data);
		rm_ScanIterator.close(rm_ScanIterator.get);
		#ifdef DEBUG
			cout<<"GET free table id: "<<tableID<<endl;
		#endif
		return tableID;
	}


	#ifdef DEBUG
		cout<<"There is bug on GetFreeTableid"<<endl;
	#endif
	return -1;

}
RC CreateVarChar(void *data,string &str){
	int size=str.size();
	int offset=0;
	memcpy((char *)data+offset,&size,sizeof(int));
	offset+=sizeof(int);
	memcpy((char *)data+offset,str.c_str,size);
	offset+=size;


	return 0;
}


RelationManager* RelationManager::_rm = 0;

RelationManager* RelationManager::instance()
{
    if(!_rm)
        _rm = new RelationManager();

    return _rm;
}

RelationManager::RelationManager()
{
}

RelationManager::~RelationManager()
{
}

RC RelationManager::createCatalog()
{
	vector<Attribute> tablesdescriptor;
	vector<Attribute> columnsdescriptor;

	RecordBasedFileManager *rbfm=RecordBasedFileManager::instance();
	FileHandle table_filehandle;
	RID rid;


	//creat Tables
	if((rbfm->createFile("Tables"))==0){

		void *data=malloc(PAGE_SIZE);
		int tableid=1;
		int systemtable=1;

		rbfm->openFile("Tables",table_filehandle);

		PrepareCatalogDescriptor("Tables",tablesdescriptor);
		CreateTablesRecord(data,tableid,"Tables",systemtable);
		rbfm->insertRecord(table_filehandle,tablesdescriptor,data,rid);

		tableid=2;
		CreateTablesRecord(data,tableid,"Columns",systemtable);
		rbfm->insertRecord(table_filehandle,tablesdescriptor,data,rid);

		rbfm->closeFile(table_filehandle);
		//create Columns
		if((rbfm->createFile("Columns"))==0){

			UpdateColumns(1,tablesdescriptor);

			PrepareCatalogDescriptor("Columns",columnsdescriptor);
			UpdateColumns(tableid,columnsdescriptor);



			free(data);
			#ifdef DEBUG
				cout<<"successfully create catalog"<<endl;
			#endif
			return 0;
		}
	}
	#ifdef DEBUG
		cout<<"Fail to create catalog"<<endl;
	#endif
    return -1;
}

RC RelationManager::deleteCatalog()
{
	RecordBasedFileManager *rbfm=RecordBasedFileManager::instance();

	if(rbfm->destroyFile("Tables")==0){
		if(rbfm->destroyFile("Columns")==0){
			#ifdef DEBUG
				cout<<"successfully delete Tables and Columns "<<endl;
			#endif
			return 0;
		}
	}
    return -1;
}

RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
	RecordBasedFileManager *rbfm=RecordBasedFileManager::instance();
	FileHandle filehandle;
	FileHandle nullhandle;
	vector<Attribute> tablesdescriptor;
	char *data=(char *)malloc(PAGE_SIZE);
	RID rid;
	int tableid;
	if(rbfm->createFile(tableName)==0){


		if(rbfm->openFile("Tables",filehandle)==0){
			tableid=GetFreeTableid();
			PrepareCatalogDescriptor("Tables",tablesdescriptor);
			CreateTablesRecord(data,tableid,tableName,0);
			rbfm->insertRecord(filehandle,tablesdescriptor,data,rid);
			#ifdef DEBUG
				cout<<"In createTable"<<endl;
				rbfm->printRecord(tablesdescriptor,data);
			#endif
			if(UpdateColumns(tableid,attrs)==0){
				free(data);
				return 0;
			}
		}

	}
	#ifdef DEBUG
		cout<<"There is bug on createTable "<<endl;
	#endif
    return -1;
}
int getTableId(const string &tableName){
	RecordBasedFileManager *rbfm=RecordBasedFileManager::instance();
	FileHandle filehandle;
	RM_ScanIterator rm_ScanIterator;
	RID rid;
	char *VarChardata=(char *)malloc(PAGE_SIZE);
	vector<string> attrname;
	attrname.push_back("table-id");

	if(RelationManager::scan("Tables","",NO_EQ,data,attrname,rm_ScanIterator)==0){
				while(rm_ScanIterator.getNextTuple(rid,data)!=RM_EOF){
					//!!!! skip null indicator
					memcpy(&foundID,(char *)data+1,sizeof(int));
					scanID[foundID-1]=true;

				}
				for(int i=0;i<TABLE_SIZE;i++){
					if(!scanID[i]){
						tableID=i+1;
						break;
					}
				}

}

RC RelationManager::deleteTable(const string &tableName)
{
	RecordBasedFileManager *rbfm=RecordBasedFileManager::instance();
	FileHandle filehandle;
	RM_ScanIterator rm_ScanIterator;
	RID rid;
	char *VarChardata=(char *)malloc(PAGE_SIZE);
	vector<string> attrname;
	attrname.push_back(tableName);

	if(rbfm->destroyFile(tableName)==0){
		if(RelationManager::scan("Tables","",NO_EQ,data,attrname,rm_ScanIterator)==0){
			while(rm_ScanIterator.getNextTuple(rid,data)!=RM_EOF){
				//!!!! skip null indicator
				memcpy(&foundID,(char *)data+1,sizeof(int));
				scanID[foundID-1]=true;

			}
			for(int i=0;i<TABLE_SIZE;i++){
				if(!scanID[i]){
					tableID=i+1;
					break;
				}
			}

			free(data);
			rm_ScanIterator.close(rm_ScanIterator.get);

	}
    return -1;
}

RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{
    return -1;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{
    return -1;
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
    return -1;
}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
    return -1;
}

RC RelationManager::readTuple(const string &tableName, const RID &rid, void *data)
{
    return -1;
}

RC RelationManager::printTuple(const vector<Attribute> &attrs, const void *data)
{
	return -1;
}

RC RelationManager::readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data)
{
    return -1;
}

RC RelationManager::scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  
      const void *value,                    
      const vector<string> &attributeNames,
      RM_ScanIterator &rm_ScanIterator)
{
    return -1;
}

// Extra credit work
RC RelationManager::addAttribute(const string &tableName, const Attribute &attr)
{
    return -1;
}

// Extra credit work
RC RelationManager::dropAttribute(const string &tableName, const string &attributeName)
{
    return -1;
}