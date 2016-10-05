#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <ctime>
using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0

uint numVars;
uint numClauses;

vector<vector<int> > clauses;
vector<int> model;
vector<int> modelStack;

uint indexOfNextLitToPropagate;
uint decisionLevel;

vector<vector<uint> > positius; //clausules on apareix positiu
vector<vector<uint> > negatius; //clausules on apareix negatiu


//first= positiu second=negatiu
vector<pair<int,int> > prior;

int start_s;
int stop_s;



void readClauses( ){
    // Skip comments
    char c = cin.get();
    while (c == 'c') {
        while (c != '\n') c = cin.get();
        c = cin.get();
    }  
    // Read "cnf numVars numClauses"
    string aux;
    cin >> aux >> numVars >> numClauses;
    clauses.resize(numClauses); 
    positius.resize(numVars+1); //la posició 0 és inútil
    negatius.resize(numVars+1); //la posició 0 és inútil
    prior.resize(numVars+1);
    
    // Read clauses
    for (uint i = 0; i < numClauses; ++i) {
        int lit;
        while (cin >> lit and lit != 0){
            clauses[i].push_back(lit);
            //afegir info de les clausules on es cada variable
            //depenent de si es positiva o negativa
            if(lit<0){
                negatius[-lit].push_back(i); ++prior[-lit].second;}
            else{
                positius[lit].push_back(i); ++prior[lit].first;}
        }    
    } 
}



int currentValueInModel(int lit){
    if (lit >= 0) return model[lit];
    else {
        if (model[-lit] == UNDEF) return UNDEF;
        else return 1 - model[-lit];
    }
}


void setLiteralToTrue(int lit){
    modelStack.push_back(lit);
    if (lit > 0) model[lit] = TRUE;
    else model[-lit] = FALSE;		
}


bool propagateGivesConflict ( ) {
    
    while ( indexOfNextLitToPropagate < modelStack.size() ) {
        int ultim = modelStack[indexOfNextLitToPropagate];
        
        if(ultim>0){ //si es positiu
            for (uint i = 0; i < negatius[ultim].size(); ++i) {
                int cl = negatius[ultim][i];
                bool someLitTrue = false;
                int numUndefs = 0;
                int lastLitUndef = 0;
                for (uint k = 0; not someLitTrue and k < clauses[cl].size(); ++k){
                    int val = currentValueInModel(clauses[cl][k]);
                    if (val == TRUE) someLitTrue = true;
                    else if (val == UNDEF){ 
                        ++numUndefs; lastLitUndef = clauses[cl][k]; }
                }
                if (not someLitTrue and numUndefs == 0){
                    prior[ultim].second = prior[ultim].second + 30;
                    return true;} // conflict! all lits false
                else if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);	
            }
        }
        else{ //si es negatiu
            for (uint i = 0; i < positius[-ultim].size(); ++i) {
                int cl = positius[-ultim][i];
                bool someLitTrue = false;
                int numUndefs = 0;
                int lastLitUndef = 0;
                for (uint k = 0; not someLitTrue and k < clauses[cl].size(); ++k){
                    int val = currentValueInModel(clauses[cl][k]);
                    if (val == TRUE) someLitTrue = true;
                    else if (val == UNDEF){ 
                        ++numUndefs; lastLitUndef = clauses[cl][k]; }
                }
                if (not someLitTrue and numUndefs == 0){
                     prior[ultim].first = prior[ultim].first + 30;
                    return true;} // conflict! all lits false
                else if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);	
            }
        }
        ++indexOfNextLitToPropagate;
    }
    return false;
}


void backtrack(){
    uint i = modelStack.size() -1;
    int lit = 0;
    while (modelStack[i] != 0){ // 0 is the DL mark
        lit = modelStack[i];
        model[abs(lit)] = UNDEF;
        modelStack.pop_back();
        --i;
    }
    // at this point, lit is the last decision
    modelStack.pop_back(); // remove the DL mark
    --decisionLevel;
    indexOfNextLitToPropagate = modelStack.size();
    setLiteralToTrue(-lit);  // reverse last decision
}


// Heuristic for finding the next decision literal:
int getNextDecisionLiteral(){
    int max = 0;
    int lit = 0;
    for (uint i = 0; i < prior.size(); ++i){
        if (i!=0 and model[i] == UNDEF){
            int t = prior[i].second + prior[i].first;
            if(t>max){
                max = t;
                lit = i;
            }
        }
    }
    if((prior[lit].first) > (prior[max].second)){
        return lit; //si esta mes cops positiva
    }
    else{
        return -lit; //si esta mes cops negativa
                
    }
        return 0; // reurns 0 when all literals are defined
}

void checkmodel(){
    for (int i = 0; i < numClauses; ++i){
        bool someTrue = false;
        for (int j = 0; not someTrue and j < clauses[i].size(); ++j)
            someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
        if (not someTrue) {
            cout << "Error in model, clause is not satisfied:";
            for (int j = 0; j < clauses[i].size(); ++j) cout << clauses[i][j] << " ";
            cout << endl;
            exit(1);
        }
    }  
}

void stop_time(){
    stop_s=clock();
    cout << "   time: " << (stop_s-start_s)/double(CLOCKS_PER_SEC) << endl;
}

int main(){ 
    start_s=clock();
    readClauses(); // reads numVars, numClauses and clauses
    model.resize(numVars+1,UNDEF);
    indexOfNextLitToPropagate = 0;  
    decisionLevel = 0;
    
    
    
    // Take care of initial unit clauses, if any
    for (uint i = 0; i < numClauses; ++i)
        if (clauses[i].size() == 1) {
            int lit = clauses[i][0];
            int val = currentValueInModel(lit);
            if (val == FALSE) {cout << "UNSATISFIABLE"; stop_time(); return 10;}
            else if (val == UNDEF) setLiteralToTrue(lit);
        }
        
        // DPLL algorithm
        while (true) {
            while ( propagateGivesConflict() ) {
                if ( decisionLevel == 0) { cout << "UNSATISFIABLE"; stop_time(); return 10; }
                backtrack();
            }
            int decisionLit = getNextDecisionLiteral();
            if (decisionLit == 0) { checkmodel(); cout << "SATISFIABLE"; stop_time(); return 20; }
            // start new decision level:
            modelStack.push_back(0);  // push mark indicating new DL
            ++indexOfNextLitToPropagate;
            ++decisionLevel;
            setLiteralToTrue(decisionLit);    // now push decisionLit on top of the mark
        }
        stop_time();
}  
