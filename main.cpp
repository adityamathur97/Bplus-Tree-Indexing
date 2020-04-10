//============================================================================
// Name        : main.cpp
// Author      : Aditya Mathur
// Net ID	   : AXM180195
// Description : Multi-level Indexing using B+ Tree.
//============================================================================

// Header Files

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <stack>
#include <cmath>

using namespace std;

/**
 * Key class This class is used to store keys in a given Node in a B+ tree. Each
 * Key has a value which is the key value and it has a pointer which is a tree
 * pointer if it is an internal node or is a data pointer if it is a leaf node.
 */

class Key{
    public:
    char *val;
    long long point;
};

/**
 * Node class This class is used to store nodes in the B+ tree. It contains the type of the node.
 */

 class Node{
    public:
    char nodeType;
    int numOfKeys;
    Key *keys;

    // This is a pointer to the next node if the node is a leaf node and is the last
    // pointer if it is an internal node
    long long nextPoint = -1;

    void init(fstream &indxFile, long long currNodePointer, int kLen, int maxNumOfKeys) {
            char *key = (char *)malloc(sizeof(*key) * kLen);
            long long point;
            indxFile.seekg(currNodePointer);
            indxFile.read(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));
            indxFile.read(reinterpret_cast<char *>(&numOfKeys), sizeof(numOfKeys));

            keys = new Key[maxNumOfKeys+1];  // one extra key in the case when the node is full
            for(int i=0; i<numOfKeys; i++){
                indxFile.read(key, kLen);
                indxFile.read(reinterpret_cast<char *>(&point), sizeof(point));
                keys[i].val = (char *) malloc(kLen);
                memcpy(keys[i].val, key, kLen);
                keys[i].point = point;
            }
            if(nodeType=='I'){
                indxFile.read(reinterpret_cast<char *>(&point), sizeof(point));
                if(numOfKeys==maxNumOfKeys){
                    nextPoint = point;
                }
                else{
                    string blank = "";
                    keys[numOfKeys].val = (char *)blank.c_str();
                    keys[numOfKeys].point=point;
                }
            }
            else if(nodeType=='L'){
                indxFile.read(reinterpret_cast<char *>(&point), sizeof(point));
                nextPoint = point;
            }
    }
 };

/**
* changeRNodePoint() changes the root node pointer in the index file.
*/
void changeRNodePoint(long long newRNodePointer, fstream &indxFile){
    indxFile.seekp(260);
    indxFile.write(reinterpret_cast<char *>(&newRNodePointer), sizeof(newRNodePointer));
}

/**
* insertKeyInLeaf() inserts the key in the leaf node in the correct position.
*/
Node insertKeyInLeaf(Node currNode, Key newKey, int maxNumOfKeys) {
    Key *keyArray = new Key[maxNumOfKeys+1];
	int currKPos = 0;
	while (currKPos < currNode.numOfKeys
			&& strcmp(currNode.keys[currKPos].val,newKey.val) < 0) {
			currKPos++;
		}
    int tempPos = 0;
	for (int j = currKPos; j < currNode.numOfKeys; j++) {
        keyArray[tempPos] = currNode.keys[j];
		tempPos++;
    }
    currNode.keys[currKPos] = newKey;
    currNode.numOfKeys++;
	tempPos = 0;
    for (int j = currKPos + 1; j < currNode.numOfKeys; j++) {
		currNode.keys[j] = keyArray[tempPos];
		tempPos++;
	}
	return currNode;
}

/**
* insertKeyInBody() inserts the key in the internal node in the correct position.
*/
Node insertKeyInBody(Node currNode,Key newKey,int maxNumOfKeys,long long nextNodePointer) {
    Key *KeyArray = new Key[maxNumOfKeys+1];
	int currKeyPos = 0;
	string blank = "";
	while (currKeyPos < currNode.numOfKeys
			&& strcmp(currNode.keys[currKeyPos].val, newKey.val) < 0) {
		currKeyPos++;
	}
	int tempPos = 0;
	for (int j = currKeyPos; j < currNode.numOfKeys; j++) {
		KeyArray[tempPos] = currNode.keys[j];
		tempPos++;
	}

	if(currNode.numOfKeys == maxNumOfKeys) {
		KeyArray[tempPos].val = (char *)blank.c_str();
		KeyArray[tempPos].point = currNode.nextPoint;
	}else {
		KeyArray[tempPos] = currNode.keys[currNode.numOfKeys];
	}
	currNode.keys[currKeyPos] = newKey;
	currNode.numOfKeys++;
	tempPos = 0;
	for (int j = currKeyPos + 1; j < currNode.numOfKeys; j++) {
		currNode.keys[j] = KeyArray[tempPos];
		tempPos++;
	}
	if(currNode.numOfKeys >= (maxNumOfKeys)) {
		currNode.nextPoint = KeyArray[tempPos].point;
	}
	else {
		currNode.keys[currNode.numOfKeys]= KeyArray[tempPos];
	}

	if((currKeyPos+1) == maxNumOfKeys) {
		currNode.nextPoint = nextNodePointer;
	}
	else if((currKeyPos+1) == currNode.numOfKeys) {
		currNode.keys[currKeyPos+1].val = (char *)blank.c_str();
		currNode.keys[currKeyPos+1].point = nextNodePointer;
	}else {
		currNode.keys[currKeyPos+1].point=nextNodePointer;
	}
	return currNode;
}


/**
* getNextNode() gets a pointer to the next available space for a new node.
*/
long long getNextNode(fstream &indxFile) {
    indxFile.seekg(0, indxFile.end);
    long long fileLen = indxFile.tellg();
    indxFile.seekg(ios::beg);

    long long nextNodePointer = -1;
	double fileLength = (double) fileLen;
	nextNodePointer = (long long) ceil(fileLength / 1024) * 1024;
	return nextNodePointer;
}

/**
* writeLeafIntoIndx() writes the leaf node into the index file.
*/
void writeLeafIntoIndx(long long currNodePointer, Node currNode, fstream &indxFile, int kLen) {
    indxFile.seekp(currNodePointer);
    indxFile.write(reinterpret_cast<char *>(&(currNode.nodeType)), sizeof(currNode.nodeType));
    indxFile.write(reinterpret_cast<char *>(&(currNode.numOfKeys)), sizeof(currNode.numOfKeys));
	for (int i = 0; i < currNode.numOfKeys; i++) {
		indxFile.write(reinterpret_cast<char *>(currNode.keys[i].val), kLen);
		indxFile.write(reinterpret_cast<char *>(&(currNode.keys[i].point)), sizeof(currNode.keys[i].point));
	}
	indxFile.write(reinterpret_cast<char *>(&(currNode.nextPoint)), sizeof(currNode.nextPoint));
}

/**
* writeInternalNodeIntoIndx() writes the internal node into the index file.
*/
void writeInternalNodeIntoIndx(long long currNodePointer, Node currNode,fstream &indxFile,int maxNumOfKeys, int kLen) {
	indxFile.seekp(currNodePointer);
	indxFile.write(reinterpret_cast<char *>(&(currNode.nodeType)), sizeof(currNode.nodeType));
	indxFile.write(reinterpret_cast<char *>(&(currNode.numOfKeys)), sizeof(currNode.numOfKeys));
	for (int i = 0; i < currNode.numOfKeys; i++) {
        indxFile.write(reinterpret_cast<char *>(currNode.keys[i].val), kLen);
		indxFile.write(reinterpret_cast<char *>(&(currNode.keys[i].point)), sizeof(currNode.keys[i].point));
	}
	if(currNode.numOfKeys == maxNumOfKeys) {
	    indxFile.write(reinterpret_cast<char *>(&(currNode.nextPoint)), sizeof(currNode.nextPoint));
	}
	else {
		indxFile.write(reinterpret_cast<char *>(&(currNode.keys[currNode.numOfKeys].point)),
                  sizeof(currNode.keys[currNode.numOfKeys].point));
	}
}

/**
* insertRecToIndx() inserts the records into the index file into the B+ tree data structure.
*/
void insertRecToIndx(long long rNodePointer, fstream &indxFile, long long dataPointer,
                           char* key, int kLen, int maxNumOfKeys){
    long long currNodePointer = rNodePointer;
    Node currNode;
    string blank = "";
    stack <long long> nodesTraversed;
    indxFile.seekg(rNodePointer);
    currNode.init(indxFile, currNodePointer, kLen, maxNumOfKeys);
    while(currNode.nodeType == 'I'){
        // loop through the B+ tree till we find the leaf node where the key is to be inserted
        nodesTraversed.push(currNodePointer);
        // Comparing with visited nodes.
        if(strcmp(key, currNode.keys[0].val)<=0){
            currNodePointer = currNode.keys[0].point;
        }
        else if(strcmp(key, currNode.keys[currNode.numOfKeys-1].val)>0){
            if(currNode.numOfKeys == maxNumOfKeys){
                currNodePointer = currNode.nextPoint;
            }else{
                currNodePointer = currNode.keys[currNode.numOfKeys].point;
            }
        } else {
            for(int i=1; i<currNode.numOfKeys; i++){
                if(strcmp(key, currNode.keys[i-1].val)>0 && (strcmp(key, currNode.keys[i].val)<=0)){
                    currNodePointer = currNode.keys[i].point;
                    break;
                }
            }
        }
        currNode.init(indxFile, currNodePointer, kLen, maxNumOfKeys);
    }

    for(int i=0; i<currNode.numOfKeys; i++){
        if(strcmp(currNode.keys[i].val, key)==0){
            cout << "Error: Duplicate key  " << key << " already exist!\n Terminating Program!" << "\n";
            return;
        }
    }

    Key newKey;
    newKey.val = key;
    newKey.point = dataPointer;
    if(currNode.numOfKeys != maxNumOfKeys){
        currNode = insertKeyInLeaf(currNode, newKey, maxNumOfKeys);
        writeLeafIntoIndx(currNodePointer, currNode, indxFile, kLen);
    }
    else{
        Node tempNode;
        tempNode.nodeType = currNode.nodeType;
        tempNode.numOfKeys = currNode.numOfKeys;
        tempNode.nextPoint = currNode.nextPoint;
        tempNode.keys = new Key[maxNumOfKeys+1];
        for(int i=0; i<currNode.numOfKeys; i++){
            tempNode.keys[i].point = currNode.keys[i].point;
            tempNode.keys[i].val = currNode.keys[i].val;
        }
        tempNode = insertKeyInLeaf(tempNode, newKey, maxNumOfKeys);
        Node newLeafNode;
        newLeafNode.nodeType = 'L';
        newLeafNode.nextPoint = currNode.nextPoint;
        newLeafNode.keys = new Key[maxNumOfKeys];
        currNode.keys = new Key[maxNumOfKeys];
        long long newLeafNodePointer = getNextNode(indxFile);
        int midPos = (int) ceil(((double)(tempNode.numOfKeys+1))/2);
        int i;
        int j=0;
        for(i=0; i<midPos; i++){
            currNode.keys[i]=tempNode.keys[i];
        }
        currNode.numOfKeys = midPos;
        currNode.nextPoint = newLeafNodePointer;
        for(i=midPos; i<tempNode.numOfKeys; i++){
            newLeafNode.keys[j] = tempNode.keys[i];
            j++;
        }
        newLeafNode.numOfKeys = j;
        writeLeafIntoIndx(currNodePointer, currNode, indxFile, kLen);
        writeLeafIntoIndx(newLeafNodePointer, newLeafNode, indxFile, kLen);
        newKey.val = tempNode.keys[midPos-1].val;
        newKey.point = currNodePointer;
        long long nextNodePointer = newLeafNodePointer;

        bool finish = false;
        while(!finish){
            if(nodesTraversed.empty()){
                Node root;
                root.nodeType = 'I';
                root.numOfKeys = 1;
                root.keys = new Key[maxNumOfKeys];
                root.keys[0].val =newKey.val;
                root.keys[0].point = newKey.point;
                root.keys[1].val = (char *)blank.c_str();;
                root.keys[1].point = nextNodePointer;
                long long newRootNodePointer = getNextNode(indxFile);
                writeInternalNodeIntoIndx(newRootNodePointer, root, indxFile, maxNumOfKeys, kLen);
                changeRNodePoint(newRootNodePointer, indxFile);
                finish = true;
            }
            else{
                currNodePointer = nodesTraversed.top();
                nodesTraversed.pop();
                Node currInternalNode;
                currInternalNode.init(indxFile, currNodePointer, kLen, maxNumOfKeys);
                if(currInternalNode.numOfKeys!=maxNumOfKeys){
                    currInternalNode = insertKeyInBody(currInternalNode, newKey, maxNumOfKeys, nextNodePointer);
                    writeInternalNodeIntoIndx(currNodePointer, currInternalNode, indxFile, maxNumOfKeys, kLen);
                    finish = true;
                }
                else{
                    Node tempInternalNode;
                    tempInternalNode.nodeType = currInternalNode.nodeType;
                    tempInternalNode.numOfKeys = currInternalNode.numOfKeys;
                    tempInternalNode.nextPoint = currInternalNode.nextPoint;
					tempInternalNode.keys = new Key[maxNumOfKeys+1];
					for(i=0;i<=currInternalNode.numOfKeys;i++) {
						tempInternalNode.keys[i]=currInternalNode.keys[i];
					}
					tempInternalNode = insertKeyInBody(tempInternalNode,newKey,maxNumOfKeys,nextNodePointer);
					Node newInternalNode;
					newInternalNode.nodeType = 'I';
                    newInternalNode.keys = new Key[maxNumOfKeys];
					currInternalNode.keys = new Key[maxNumOfKeys];
					long long newInternalNodePointer = getNextNode(indxFile);
					midPos = (int) floor(((double)(tempInternalNode.numOfKeys+1))/2);
                    currInternalNode.numOfKeys = midPos;
                    for(i=0;i<midPos;i++) {
						currInternalNode.keys[i].val = tempInternalNode.keys[i].val;
						currInternalNode.keys[i].point = tempInternalNode.keys[i].point;
					}
					currInternalNode.keys[midPos].val = (char *)blank.c_str();;
					currInternalNode.keys[midPos].point = tempInternalNode.keys[midPos].point;
                    j=0;
					for(i=midPos+1;i<tempInternalNode.numOfKeys;i++) {
						newInternalNode.keys[j].val = tempInternalNode.keys[i].val;
						newInternalNode.keys[j].point = tempInternalNode.keys[i].point;
						j++;
					}
					newInternalNode.numOfKeys = j;
                    newInternalNode.keys[j].val = (char *)blank.c_str();;
                    newInternalNode.keys[j].point = tempInternalNode.nextPoint;
					writeInternalNodeIntoIndx(currNodePointer, currInternalNode, indxFile,maxNumOfKeys, kLen);
					writeInternalNodeIntoIndx(newInternalNodePointer, newInternalNode, indxFile,maxNumOfKeys, kLen);
					newKey.val = tempInternalNode.keys[midPos].val;
					newKey.point = currNodePointer;
					nextNodePointer = newInternalNodePointer;

                }
            }
        }
    }
}

/**
* getRNodePointer() returns the root node pointer.
*/
long long getRNodePointer(fstream &indxFile){
    long long pointer = -1;
    indxFile.seekg(260);
    indxFile.read(reinterpret_cast<char *>(&pointer), sizeof(pointer));
    return pointer;
}

/**
* crtIndx() is used to create an index on the given text file.
*/
int crtIndx(char *txtFileName, char *indxFileName, int kLen) {
    fstream txtFile, indxFile;
    txtFile.open(txtFileName, ios::in|ios::binary);     // access the input text file

    indxFile.open(indxFileName, ios::in|ios::out|ios::trunc|ios::binary);

	indxFile.write(reinterpret_cast<char *>(txtFileName), strlen(txtFileName));   // writing the name of the text file into the header node

	indxFile.seekp(256);  // Move 256 bytes from the start of the file

	int metaDataLen = 6; // meta data length in bytes
    int maxNumOfKeys = (1024 - metaDataLen - 8) / (8 + kLen);
	indxFile.write(reinterpret_cast<char *>(&maxNumOfKeys), sizeof(maxNumOfKeys));
	long long rNodePointer = 1024; // The first node is after 1K bytes
    indxFile.write(reinterpret_cast<char *>(&rNodePointer), sizeof(rNodePointer));
	indxFile.write(reinterpret_cast<char *>(&kLen), sizeof(kLen));

    indxFile.seekp(rNodePointer);
    char l = 'L';
	indxFile.write(reinterpret_cast<char *>(&l), sizeof(l));
    int temp1 = 1;
    indxFile.write(reinterpret_cast<char *>(&temp1), sizeof(temp1));
	long long dataPointer = txtFile.tellg();
	string data = "";
	getline(txtFile, data);
	char *key = (char *)malloc(sizeof(*key) * kLen);
	memcpy(key, &data[0], kLen);
    indxFile.write(reinterpret_cast<char *>(key), kLen);
    indxFile.write(reinterpret_cast<char *>(&dataPointer), sizeof(dataPointer));
    long long temp2 = -1;
	indxFile.write(reinterpret_cast<char *>(&temp2), sizeof(temp2));
	dataPointer = txtFile.tellg();
    while (getline(txtFile, data)) {
        memcpy(key, &data[0], kLen);
        rNodePointer = getRNodePointer(indxFile);
        insertRecToIndx(rNodePointer, indxFile, dataPointer, key, kLen,
						maxNumOfKeys);
        dataPointer = txtFile.tellg();
    }
    free(key);
    txtFile.close();
    indxFile.close();
    return 1;
}

/**
 * findRec() finds the position of a record in the text file.
 */

int findRec(char* indxFileName, char* kVal){
    fstream indxFile, txtFile;
    indxFile.open(indxFileName, ios::in | ios::binary);
    char *refByte = (char *)malloc(256);
    indxFile.seekg(0);
    indxFile.read(refByte, 256);

    txtFile.open(refByte, ios::in | ios::binary);

    int maxNumOfKeys;
    indxFile.read(reinterpret_cast<char *>(&maxNumOfKeys), sizeof(maxNumOfKeys));

    long long rNodePointer;
    indxFile.read(reinterpret_cast<char *>(&rNodePointer), sizeof(rNodePointer));

    int kLen;
    indxFile.read(reinterpret_cast<char *>(&kLen),sizeof(kLen));

    long long currNodePointer = rNodePointer;

    Node currNode;
    bool found = false;

    int kPos = -1;

    if(strlen(kVal)>kLen){
        string temp(kVal);
        temp = temp.substr(0,kLen);
        kVal = (char*)temp.c_str();

    }else if(strlen(kVal) < kLen){
        int lengthDiff = kLen - strlen(kVal);
        string temp(kVal);
        for(int i=0; i<lengthDiff; i++){
            temp = temp + " ";
        }
        kVal = (char*)temp.c_str();
    }

    indxFile.seekg(rNodePointer);
    currNode.init(indxFile, currNodePointer, kLen, maxNumOfKeys);
    while(currNode.nodeType != 'L'){
        if(strcmp(kVal,currNode.keys[0].val) <= 0){
            currNodePointer = currNode.keys[0].point;
        }else if(strcmp(kVal,currNode.keys[currNode.numOfKeys-1].val) >0){
            if(currNode.numOfKeys == maxNumOfKeys){
                currNodePointer = currNode.nextPoint;
            }else{
                currNodePointer = currNode.keys[currNode.numOfKeys].point;
            }
        }else{
            for(int i=1; i<currNode.numOfKeys; i++){
                if((strcmp(kVal,currNode.keys[i-1].val) > 0) && (strcmp(kVal,currNode.keys[i].val) <=0)){
                    currNodePointer = currNode.keys[i].point;
                    break;
                }
            }
        }
        currNode.init(indxFile, currNodePointer, kLen, maxNumOfKeys);
    }

    for (int i = 0; i < currNode.numOfKeys; i++) {
        if (strcmp(currNode.keys[i].val,kVal) == 0) {
            found = true;
            kPos = i;
            break;
        }
    }
    if(found) {
        long long recPointer = currNode.keys[kPos].point;
        txtFile.seekg(recPointer);
        string record;
        getline(txtFile,record);
        cout<<"At position "<<recPointer<<" the record is :-"<<record;
    }else {
        cout<<"Message: Record with the given key not found!";
    }
    txtFile.close();
    indxFile.close();

    return 1;
}

/**
 * insertRecToTextFile() inserts record into the text file.
 */

int insertRecToTextFile(char* indxFileName, char* newRec){
    fstream indxFile, txtFile;
    indxFile.open(indxFileName, ios::in|ios::out|ios::binary);

    char* refByte = (char *)malloc(256);
	indxFile.seekg(0);
	indxFile.read(refByte, 256);

	txtFile.open(refByte, ios::in|ios::app);

	int maxNumOfKeys;
	indxFile.read(reinterpret_cast<char *>(&maxNumOfKeys), sizeof(maxNumOfKeys));

	long long rNodePointer;
	indxFile.read(reinterpret_cast<char *>(&rNodePointer), sizeof(rNodePointer));

	int kLen;
	indxFile.read(reinterpret_cast<char *>(&kLen), sizeof(kLen));

	long long currNodePointer = rNodePointer;
	Node currNode;
	bool found = false;

	char* kVal = (char *)malloc(kLen);
	memcpy(kVal, newRec, kLen);
	indxFile.seekg(rNodePointer);
	currNode.init(indxFile, currNodePointer, kLen, maxNumOfKeys);
	while (currNode.nodeType=='I') {
		if (strcmp(kVal, currNode.keys[0].val) <= 0) {
			currNodePointer = currNode.keys[0].point;
		} else if (strcmp(kVal,currNode.keys[currNode.numOfKeys-1].val) > 0) {
			if(currNode.numOfKeys == maxNumOfKeys) {
				currNodePointer = currNode.nextPoint;
			}else {
				currNodePointer = currNode.keys[currNode.numOfKeys].point;
			}
		} else {
			for (int i = 1; i < currNode.numOfKeys; i++) {
				if ((strcmp(kVal, currNode.keys[i - 1].val) > 0)
						&& (strcmp(kVal, currNode.keys[i].val) <= 0)) {
					currNodePointer = currNode.keys[i].point;
					break;
				}
			}
		}
		currNode.init(indxFile, currNodePointer, kLen, maxNumOfKeys);
	}

	for (int i = 0; i < currNode.numOfKeys; i++) {
		if (strcmp(currNode.keys[i].val,kVal) == 0) {
			found = true;
			break;
		}
	}
	if(found) {
		cout << "Record with the provided key already exist!";
	}
	else {
		txtFile.seekg(0, txtFile.end);
		long long txtFileLen = txtFile.tellg();
		char* newLine = "\n";
		txtFile.seekp(txtFileLen);
		txtFile.write(reinterpret_cast<char *>(newRec), strlen(newRec));
		txtFile.write(reinterpret_cast<char *>(newLine), strlen(newLine));
		cout << "Message: New record successfully inserted at position " << txtFileLen << " in the text file!";
		insertRecToIndx(rNodePointer,indxFile,txtFileLen,kVal,kLen,maxNumOfKeys);
	}
	txtFile.close();
	indxFile.close();
    return 1;
}

/**
 * listSeqRec() lists specified number of records after the provided key.
 */
int listSeqRec(char* indxFileName, char* kVal, int numOfRec){
    fstream indxFile, txtFile;

    indxFile.open(indxFileName, ios::in | ios::binary);
    char *refByte = (char *)malloc(256);
    indxFile.seekg(0);
    indxFile.read(refByte, 256);

    txtFile.open(refByte, ios::in | ios::binary);

    int maxNumOfKeys;
    indxFile.read(reinterpret_cast<char *>(&maxNumOfKeys), sizeof(maxNumOfKeys));

    long long rNodePointer;
    indxFile.read(reinterpret_cast<char *>(&rNodePointer), sizeof(rNodePointer));

    int kLen;
    indxFile.read(reinterpret_cast<char *>(&kLen),sizeof(kLen));

    long long currNodePointer = rNodePointer;

    Node currNode;
    bool found = false;

    int kPos = -1;

    indxFile.seekg(rNodePointer);
    currNode.init(indxFile, currNodePointer, kLen, maxNumOfKeys);
    while(currNode.nodeType != 'L'){
        if(strcmp(kVal,currNode.keys[0].val) <= 0){
            currNodePointer = currNode.keys[0].point;
        }else if(strcmp(kVal,currNode.keys[currNode.numOfKeys-1].val) >0){
            if(currNode.numOfKeys == maxNumOfKeys){
                currNodePointer = currNode.nextPoint;
            }else{
                currNodePointer = currNode.keys[currNode.numOfKeys].point;
            }
        }else{
            for(int i=1; i<currNode.numOfKeys; i++){
                if((strcmp(kVal,currNode.keys[i-1].val) > 0) && (strcmp(kVal,currNode.keys[i].val) <=0)){
                    currNodePointer = currNode.keys[i].point;
                    break;
                }
            }
        }
        currNode.init(indxFile, currNodePointer, kLen, maxNumOfKeys);
    }

    for (int i = 0; i < currNode.numOfKeys; i++) {
        if (strncmp(currNode.keys[i].val,kVal,kLen) == 0) {
            found = true;
            kPos = i;
            break;
        }
    }


    if(!found){
        cout<<"Message: Record with given key was not found! Searching for the key greater than the provided key!";
        for(int i = 0; i < currNode.numOfKeys; i++) {
            if (strcmp(currNode.keys[i].val,kVal) > 0) {
                found = true;
                kPos = i;
                break;
            }
            if(i == (currNode.numOfKeys-1)) {
                if(currNode.nextPoint!=-1) {
                    currNode.init(indxFile, currNode.nextPoint, kLen, maxNumOfKeys);
                    i=0;
                }
            }
        }
    }
    if(!found) {
        cout<<"Message: Record with the provided key or a key greater than the provided key does not exist!";
    }else {
        for(int i=0;i<numOfRec;i++) {
            long long recPointer = currNode.keys[kPos].point;
            txtFile.seekg(recPointer);
            string record = "";
            getline(txtFile, record);
            cout<<"At position "<<recPointer<<" , record :- "<<record<<"\n";
            if(kPos == (currNode.numOfKeys-1)) {
                if(currNode.nextPoint!=-1) {
                    currNode.init(indxFile, currNode.nextPoint, kLen, maxNumOfKeys);
                    kPos=0;
                }else {
                    break;
                }
            }else {
                kPos++;
            }
        }
    }

    txtFile.close();
    indxFile.close();

    return 1;
}


int main(int argc, char *argv[])
{
    string opp = argv[1]; // The first argument is the operation
    if(opp=="-create"){
        if(argc!=5){
            cout << "Error: Wrong number of arguments passed!" << "\n";
            return 0;
        }
        char *txtFile = argv[2];  // The second argument is the input text file path/name
        char *indxFile = argv[3];  // The third argument is the output index file path/name
        int kLen = atoi(argv[4]);  // The fourth argument is the key length in bytes
        if(kLen<1 || kLen>40){
            cout << "Error: Wrong number of bytes for key!" << "\n";
            return 0;
        }
        crtIndx(txtFile, indxFile, kLen);
        cout<<"Message: Index File Created!";
    }
    else if(opp=="-find"){
        if(argc!=4){
            cout << "Error: Wrong number of arguments passed!" << "\n";
            return 0;
        }
        char* indxFileName = argv[2];
        char* kVal = argv[3];
        findRec(indxFileName, kVal);
    }
    else if(opp=="-insert"){
        if(argc!=4){
            cout << "Error: Invalid number of arguments passed!" << "\n";
            return 0;
        }
        char* indxFileName = argv[2];
        char* newRec = argv[3];
        insertRecToTextFile(indxFileName, newRec);
    }
    else if(opp=="-list"){
        if(argc!=5){
            cout << "Error: Invalid number of arguments passed!" << "\n";
            return 0;
        }
        char* indxFileName = argv[2];
        char* startKey = argv[3];
        int numOfRec = atoi(argv[4]);
        listSeqRec(indxFileName, startKey, numOfRec);
    }
    else{
        cout << "Error: Invalid input!" << "\n";
        return 0;
    }
    return 1;
}
