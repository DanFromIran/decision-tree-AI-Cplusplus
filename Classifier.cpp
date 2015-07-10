#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <cstring>
#include <stdlib.h>
#include <math.h>
#include <list>
#include <algorithm>
using namespace std; 

vector<string> wordLookup;


map<int, list<int> > getClassificationMap(string fileName)
{
	map<int, list<int> > classificationMap;
	ifstream myfile(fileName.c_str());
	if(myfile.is_open())
	{
		while(!myfile.eof())
		{
			string line; 
			getline(myfile,line );
			istringstream iss(line);
			string sub; 
			iss >> sub; 
			//first string in line is docID
			int docID = atoi(sub.c_str());
			int wordID;
				while(iss)
				{
					iss >> sub; 
				    wordID = atoi(sub.c_str());
				}
				

			if(classificationMap.find(docID)==classificationMap.end())
			{
				classificationMap[docID]=list<int>();
				classificationMap[docID].push_front(wordID);
			}
			else
			{
				classificationMap[docID].push_front(wordID);
			}
			
		}
		myfile.close();
	}
	else
	{
		cout<<"ERROR: could not open TrainData.txt\n";
	}
	return classificationMap;
}

map<int, int> getLabelMap(string fileName)
{
	map<int, int> docTypes;
	ifstream myfile(fileName.c_str()); 
	if(myfile.is_open())
	{
		int lineNumber = 1;
		while(!myfile.eof())
		{
		string line;
		getline(myfile, line);
		
		//duplicates keys should never happen so we dont have to check
		docTypes[lineNumber] = atoi(line.c_str());
		lineNumber++;
	    }
		
	myfile.close();
	}
	else
	{
		cerr<< "ERROR: coudlnt open trainLabel.txt\n";
	}	
	return docTypes;
}

class decisionTree
{
	private:
		int depth; //depth of leaf nodes is 0
		list<int> allDocs;
		
		decisionTree  * rightChild; // contains seperator word
		decisionTree  * leftChild; //does not contain seperator word
		char category; //only applicable to leaf nodes
		int seperator; //index of word that seperates documents to those containing, only applicable to non leaf nodes
		
		double iVal(int pp, int nn)
		{
			if(pp==0 || nn==0) return 0;
			double p= (double) pp;
			double n = (double) nn;
			
			double inside = p/(p+n);
			double lval =  ((-1 * p)/(p+n) * log2(p/(p+n)) );
	
	
			double result = ((-1 * p)/(p+n) * log2(p/(p+n)) ) - ( n / (p+n) * log2(n/(p+n)) );
				if(isnan(result)) 
				{
					cout<<"iVal is NaN becuase p: "<< p << " n: "<<n<<endl;
					return 0;
				}
 			return result;
		}

		list<int> getComplement(list<int> docs, list<int> total)
		{
			list<int> answer = list<int> (total);
		
			for(list<int>::iterator it = docs.begin(); it != docs.end(); it++)
			{
				answer.remove( *it );
			}
	
			return answer;
		}

		double getIG( int Gc, int Ac, int Gn, int An )
		{
			int total = Gc+Ac+Gn+An;
				if(total==0) {
				//	cout<<"total was 0\n";
					return 0;
				}
			double leftClause = ( (double) (Gc+Ac) / (double) total) * iVal(Gc,Ac);
			double rightClause = ( (double) (Gn+An) / (double) total) * iVal(Gn,An);
			return iVal(Gc+Gn, Ac+An) - leftClause - rightClause;
		}
	
		char classify(list<int> words)
		{
			if(depth==0)
			{
				if((category!= 'A') && (category!='G')) cerr<< " leaf node has invalid category\n";
				return category; 
			}
			list<int>::iterator it = find(words.begin(), words.end(), seperator);
			
			if(it==words.end())
			{
				return leftChild->classify(words);
			}
			else
			{
				return rightChild->classify(words);
			}
			
		}
		
		void init (map<int, list<int> > trainMap, map<int,int> docTypes) 
		{
		double maxIG=-10; //dummy low value
		int bestWord=1;
	
		if(depth > 0)
		{
		
			for(map<int, list<int> >::iterator it = trainMap.begin(); it!= trainMap.end(); it++)
			{
				
					int Gc=0; //number of documents that include thw word and are graphics related
					int Ac=0; //number of documents that include the word and are atheism related
					int Gn=0; //number of documents that do not include the word and are graphics related
					int An=0; //number of documents that do not include the word and are atheism related	
					int potentialSeperator = it->first;
					
					//following loop will not run if there is a word that does not appear in any documents 
					for(list<int>::iterator it2 = it->second.begin(); it2!= it->second.end(); it2++)
					{
						
						if(docTypes.find(*it2) != docTypes.end())
						{
							if(docTypes[*it2]==2) //doc is graphics
							{
								Gc++;
							}
							else if (docTypes[*it2]==1 )//doc is atheist
							{
								Ac++;
							}
							else
							{
								cerr<<"label for doc# " << *it2 <<" is not 2 or 1\n";
							}
						}
						else
						{
						cerr<<"could not find label for document# "	<< *it2 <<endl;
						}
					}
					
					list<int> complement = getComplement(it->second, allDocs);
					
					
					for(list<int>::iterator it2 = complement.begin(); it2!=complement.end(); it2++)
					{
						if(docTypes.find(*it2) != docTypes.end())
						{
							if(docTypes[*it2]==2) //doc is graphics
							{
								Gn++;
							}
							else if (docTypes[*it2]==1)//doc is atheist
							{
								An++;
							}
							else
							{
								cerr<<"label for doc# " << *it2 <<" is not 2 or 1\n";
							}
						}
						else
						{
						cerr<<"could not find label for document# "	<< *it2 <<endl;
						}
					}
					
					//now that we have the list of all documents that contains and all documents that dont contain the word,
					//we can calculate the information gain.
					
					double	newIG = getIG(Gc, Ac, Gn, An);
					if(isnan(newIG))
					{
						cout<<"newIG is NaN because Gc: " << Gc<<" Ac: "<<Ac << " Gn: "<<Gn << " An:"<<An<<endl;
					}
					if(newIG > maxIG)
					{
						bestWord = it->first;
						maxIG = newIG;
						
					
					}
			}
			
			seperator=bestWord;
			 cout<<" : depth: "<< depth <<
			 " IG: "<<maxIG<< "  "<<wordLookup.at(bestWord - 1) <<endl;
			//now we need to create left and right subtrees, but in order to do that we need to update some values accordingly.
			
			
			list<int> rightTotalDocs=trainMap[bestWord];
			list<int> leftTotalDocs= getComplement(rightTotalDocs, allDocs);
			
			int Gc=0; //number of documents that include thw word and are graphics related
			int Ac=0; //number of documents that include the word and are atheism related
			int Gn=0; //number of documents that do not include the word and are graphics related
			int An=0; //number of documents that do not include the word and are atheism related	
			
			for(list<int>::iterator it= rightTotalDocs.begin(); it!= rightTotalDocs.end(); it++)
			{
				if(docTypes[*it]==1) Ac++;
				else if (docTypes[*it]==2) Gc++;
				else
				{
					cerr<<"error: wrong value returned from docTypes\n";
				}
			}
			
			for(list<int>::iterator it= leftTotalDocs.begin(); it!= leftTotalDocs.end(); it++)
			{
				if(docTypes[*it]==1) An++;
				else if (docTypes[*it]==2) Gn++;
				else
				{
					cerr<<"error: wrong value returned from docTypes\n";
				}
			}
			
		
			
			//now we need to filter the trainMap for the child nodes. 
			
			map< int, list<int> > rightMap = map<int, list<int> > ();
			map< int, list<int> > leftMap = map<int, list<int> > ();
			
			for(map<int, list<int> >::iterator it = trainMap.begin(); it!= trainMap.end() ; it++)
			{
				if(it->first != seperator) //if we already seperated by a word, we dont want that word in the map anymore.
				{
				
				list<int> filteredListRight = list<int>(); //only put in documents that are in right total
				list<int> filteredListLeft = list<int>();
			
					for(list<int>::iterator it2=it->second.begin(); it2!=it->second.end(); it2++)
					{
						list<int>::iterator it3= find(rightTotalDocs.begin(),rightTotalDocs.end() ,*it2);
						if(it3!=rightTotalDocs.end())
						{
							filteredListRight.push_front(*it2);
						}
						else
						{
							//all documents in trainMap are aither in left or right subtree, if it aint 
							//in right subtree, it is in the left
							filteredListLeft.push_front(*it2);
						}
					}
					//if(it->first == 153 ) cout<<"im about to put "<< wordLookup.at(it->first -1)<<" where seperator is: "<< wordLookup.at(seperator-1) <<endl;
					rightMap[it->first]=filteredListRight;
					leftMap[it->first] = filteredListLeft;
				}
			}
		//now that we have rightMap and leftMap as well as their totals, we can create the left and right children.
		
		
		
			//special case: only one of a type because seperator is too good.
			if(Ac==0 || Gc==0) //can both be 0?
			{
				if(Ac==0)
				{
					rightChild = new decisionTree(0,rightMap,rightTotalDocs,docTypes,'G');
					leftChild = new decisionTree(depth-1,leftMap, leftTotalDocs,docTypes);
				}
				else if (Gc==0)
				{
					rightChild = new decisionTree(0,rightMap,rightTotalDocs, docTypes, 'A');
					leftChild = new decisionTree(depth-1,leftMap, leftTotalDocs,docTypes);
				}
				return;
			}
			if(An==0 || Gn==0) //can both be 0?
			{
				if(An==0)
				{
					leftChild = new decisionTree(0,leftMap,leftTotalDocs, docTypes, 'G');
					rightChild = new decisionTree(depth-1,rightMap, rightTotalDocs, docTypes);
				}
				else if (Gn==0)
				{
					leftChild = new decisionTree(0,leftMap,leftTotalDocs, docTypes, 'A');
					rightChild = new decisionTree(depth-1,rightMap, rightTotalDocs, docTypes);
				}
				return;
			}
		
		rightChild = new decisionTree(depth-1,rightMap, rightTotalDocs, docTypes);
		leftChild = new decisionTree(depth-1,leftMap, leftTotalDocs, docTypes);
			
		}
		else if(depth==0)
		{
			//we need to set the category;
			int Gtot=0;
			int Atot=0;
			
			for(list<int>::iterator it = allDocs.begin(); it!= allDocs.end(); it++)
			{
				if (docTypes[*it] == 1 ) Atot++;
				else if(docTypes[*it] == 2) Gtot++;
				else cerr<<"ERROR: document "<< *it <<" invalid label\n";
			}
			
			if(Gtot>Atot) category = 'G';
			else category = 'A';
		}
		else
		{
				cerr<<"wrong depth value in decisionTree constructor\n";
		}
	}
	public:
			
		decisionTree (int depth, map<int, list<int> > tMap, list<int> docList,map<int,int> docTypes)
		{
			this->depth = depth; 
			this->rightChild= NULL;
			this->leftChild=NULL;
			
			category = 'X' ; //indicates a non leaf node 'A' denotes a atheist leaf node 'G' indicates a graphics leaf node
			allDocs=docList;
			init(tMap, docTypes);
		}
		
		decisionTree (int depth, map<int, list<int> > tMap, list<int> docList, map<int,int> docTypes, char cat)
		{	
			this->depth = depth; 
			this->rightChild= NULL;
			this->leftChild=NULL;
			category = cat; //indicates a non leaf node 'A' denotes a atheist leaf node 'G' indicates a graphics leaf node
			allDocs=docList;
			init(tMap,docTypes);
		}
		
		
		~decisionTree ()
		{
			if(NULL!= rightChild)
			{
				delete(rightChild);
			}
				if(NULL!= leftChild)
			{
				delete(leftChild);
			}
			
		}
		
		double getAccuracy(map<int, list<int> > inputMap, map<int, int> testLabel)
		{
			//inputMap is a map of documents to 
			int correct=0;
			int total=0;
			for(map<int, list<int> >::iterator it= inputMap.begin(); it!= inputMap.end() ; it++ )
			{
				char guess = classify(it->second);
				char actual;
				if(testLabel[it->first] ==1 )//atheism
				{
					actual='A';
				}
				else if(testLabel[it->first] ==2) //graphics
				{
				    actual='G';
				}
				else
				{
						cerr<<"invalid number gotten out of docTypes\n";
				}
				if(actual==guess)correct++;
				
				total++;
				
			}
			
			
			return ( (double) correct ) / ( (double) total ) ;
		}
	void printTree(int spaces)
	{
		for(int i=0; i<spaces; i++)cout<<"    ";
		cout<<category<<"\n\n";
		
		if(NULL != rightChild )rightChild->printTree(spaces+1);
		if(NULL != leftChild )leftChild->printTree(spaces+1);
		
		
	}
	
};




int main(){
	//trainMap is a map of word IDs to a list of all documents that contain it
	map< int,  list<int> > trainMap ;
	
	ifstream myfile("TrainData.txt");
	if(myfile.is_open())
	{
		while(!myfile.eof())
		{
			string line; 
			getline(myfile,line );
			istringstream iss(line);
			string sub; 
			iss >> sub; 
			//first string in line is docID
			int docID = atoi(sub.c_str());
			int wordID;
				while(iss)
				{
					iss >> sub; 
				    wordID = atoi(sub.c_str());
				}
				
			//now that we have docID and wordID, lets update the training map
			if(trainMap.find(wordID) == trainMap.end())
			{
				//cout<<i <<" was not found\n"<<endl;
				trainMap[wordID] = list<int>();
				trainMap[wordID].push_front(docID);		 
			}
			else // an entry already exists
			{
				
				trainMap[wordID].push_front(docID);
			}	
		}
		myfile.close();
	}
	else
	{
		cout<<"ERROR: could not open TrainData.txt\n";
	}
	

	

	
	//read in the word values
	wordLookup = vector<string> ();
	
	ifstream myfile3("words.txt");
	if(myfile3.is_open())
	{
		while (!myfile3.eof())
		{	
			string line;
			getline(myfile3,line);
			wordLookup.push_back(line);
		}
		myfile3.close();
	}
	else
	{
		cerr<<"ERROR: couldnt open words.txt";
	}
	
	map<int, int> docTypes = getLabelMap("TrainLabel.txt");
	
	list<int> docList= list <int> (); //documents are numbered 1 to 1061
	int ctr = 1;
	for(int i=0; i<docTypes.size(); i++)
	{
		docList.push_front(ctr);
		ctr++;
	}
	
	map<int, int> testingTypes = getLabelMap("TestLabel.txt");
	map<int, list<int> > trainingClassificationMap = getClassificationMap("TrainData.txt");
	map<int, list<int> > testingClassificationMap = getClassificationMap("TestData.txt");
		
	decisionTree dt = decisionTree(4, trainMap,docList,docTypes);
	double trainingAccuracy = dt.getAccuracy(trainingClassificationMap,docTypes);
	double testingAccuracy = dt.getAccuracy(testingClassificationMap,testingTypes);
	cout<<"training accuracy: "<<trainingAccuracy<<endl;
	cout<<"testing accuracy: "<<testingAccuracy<<endl;
	
	cout<<"\n\n\n";
	dt.printTree(0);
	
}//end of main function
