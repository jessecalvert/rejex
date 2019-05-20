#if !defined(FINITE_AUTOMATA_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jesse Calvert $
   ======================================================================== */

global_variable char Digits[10] = {'0','1','2','3','4','5','6','7','8','9'};
global_variable char Letters[52] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
                                    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

#define EPSILON 127
#define INPUT_COUNT 128

struct automata_node;
struct automata_transition
{
    char Input;
    automata_node *TransitionNode;

    automata_transition *Next;
};

enum node_flag
{
    NodeFlag_Start = (1 << 0),
    NodeFlag_Accept = (1 << 1),
};

struct node_list;
struct automata_node
{
    uint32 Flags;
    
    automata_transition *Transitions;

    // NOTE: used to convert DFA into dfa_table
    uint32 ID;

    //NOTE: used to convert NFA into DFA
    node_list *NFAList;
};

struct dfa_table
{
    uint32 Start;
    uint32 EndStatesCount;
    uint32 *EndStates;
    uint32 StatesCount;

    // NOTE: state index 0 is reserved for the null state.
    uint32 *Table;
};

struct node_list
{
    automata_node *Node;
    node_list *Next;    
};

struct node_pair
{
    automata_node *Original;
    automata_node *Copy;
};

//
//
//

inline void
SetFlags(automata_node *Node, uint32 Flags)
{
    Node->Flags |= Flags;
};

inline void
ClearFlags(automata_node *Node, uint32 Flags)
{
    Node->Flags &= ~Flags;
};

inline bool32
IsSet(automata_node *Node, uint32 Flags)
{
    bool32 Result = Node->Flags & Flags;
    return Result;
};

inline bool32
IsEndState(dfa_table *DFA, uint32 State)
{
    bool32 Result = false;

    for(uint32 EndStateIndex = 0;
        EndStateIndex < DFA->EndStatesCount;
        ++EndStateIndex)
    {
        if(State == DFA->EndStates[EndStateIndex])
        {
            Result = true;
            break;
        }
    }

    return Result;
}

inline uint32
GetNextState(dfa_table *DFA, uint32 State, uint8 Input)
{
    uint32 Result = *(DFA->Table + INPUT_COUNT*State + Input);
    return Result;
}

inline bool32
IsDigit(char Test)
{
    bool32 Result = false;

    for(uint32 DigitIndex = 0;
        !Result && (DigitIndex < ArrayCount(Digits));
        ++DigitIndex)
    {
        if(Digits[DigitIndex] == Test)
        {
            Result = true;
        }
    }

    return Result;
}

inline uint32
GetDigitIndex(char Digit)
{
    uint32 Result = 0;

    for(uint32 DigitIndex = 0;
        DigitIndex < ArrayCount(Digits);
        ++DigitIndex)
    {
        if(Digits[DigitIndex] == Digit)
        {
            Result = DigitIndex;
        }
    }

    return Result;
}

inline bool32
IsLetter(char Test)
{
    bool32 Result = false;

    for(uint32 LetterIndex = 0;
        !Result && (LetterIndex < ArrayCount(Letters));
        ++LetterIndex)
    {
        if(Letters[LetterIndex] == Test)
        {
            Result = true;
        }
    }

    return Result;    
}

inline uint32
GetLetterIndex(char Letter)
{
    uint32 Result = 0;

    for(uint32 LetterIndex = 0;
        LetterIndex < ArrayCount(Letters);
        ++LetterIndex)
    {
        if(Letters[LetterIndex] == Letter)
        {
            Result = LetterIndex;
        }
    }

    return Result;
}

#define FINITE_AUTOMATA_H
#endif
