/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jesse Calvert $
   ======================================================================== */

internal bool32
RunFiniteAutomata(automata_node *Start, char *Input)
{
    // NOTE: must be a DFA
    bool32 Result = false;

    automata_node *CurrentState = Start;
    for(automata_transition *Transition = Start->Transitions;
        Transition && (*Input != '\0');
        )
    {
        if(Transition->Input == *Input)
        {
            CurrentState = Transition->TransitionNode;
            Transition = CurrentState->Transitions;
            Input++;
        }
        else
        {
            Transition = Transition->Next;
        }
    }

    if(IsSet(CurrentState, NodeFlag_Accept) && (*Input == '\0'))
    {
        Result = true;
    }
    
    return Result;
}

internal bool32
RunFiniteAutomata(dfa_table *DFA, char *Input)
{
    bool32 Result = false;
    
    uint32 State = DFA->Start;
    uint32 InputIndex = 0;
    while((Input[InputIndex] != '\0') && State)
    {
        State = GetNextState(DFA, State, Input[InputIndex++]);
    }
    Result = IsEndState(DFA, State);
    
    // cout << Input << "\t" << Result << endl;

    return Result;
}

internal dfa_table
ConvertToDFATable(memory_arena *Arena, automata_node *Start)
{
    // NOTE: Start must be the first node in a DFA
    dfa_table Result = {};

    uint32 Table[1024][INPUT_COUNT] = {};

    Result.EndStatesCount = 0;

    Result.Start = 1;
    Result.StatesCount = 2;
    uint32 EndStates[1024] = {};
    
    Start->ID = 1;
    automata_node *NodeQueue[1024] = {Start};
    uint32 QueueIndex = 0;
    uint32 QueueSize = 1;

    while(QueueSize > 0)
    {

        automata_node *CurrentNode = NodeQueue[QueueIndex];
        if(IsSet(CurrentNode, NodeFlag_Accept))
        {
            EndStates[Result.EndStatesCount++] = CurrentNode->ID;
        }

        for(automata_transition *Transition = CurrentNode->Transitions;
            Transition;
            Transition = Transition->Next)
        {
            automata_node *NextNode = Transition->TransitionNode;
            
            if(NextNode->ID == 0)
            {
                NextNode->ID = Result.StatesCount++;
                NodeQueue[QueueIndex + QueueSize++] = NextNode;
            }

            *(((uint32 *)Table) + INPUT_COUNT*CurrentNode->ID + Transition->Input) = NextNode->ID;
        }

        QueueSize--;
        QueueIndex++;
    }

    Result.Table = PushArray(Arena, Result.StatesCount*INPUT_COUNT, uint32);
    CopyMemory(Table, Result.Table, Result.StatesCount*INPUT_COUNT*4);
    
    Result.EndStates = PushArray(Arena, Result.EndStatesCount, uint32);
    CopyMemory(EndStates, Result.EndStates, Result.EndStatesCount*4);
    
    return Result;
}

internal void
BuildTransition(memory_arena *Arena, automata_node *From, automata_node *To, char Input)
{
    automata_transition *Transition = PushStruct(Arena, automata_transition);
    
    Transition->Input = Input;
    Transition->TransitionNode = To;

    if(From->Transitions)
    {
        automata_transition *Temp = From->Transitions;
        From->Transitions = Transition;
        Transition->Next = Temp;
    }
    else
    {
        From->Transitions = Transition;
        Transition->Next = 0;
    }
}

internal automata_node *
GenerateDFAForWord(memory_arena *Arena, char *Word)
{
    uint32 LetterIndex = 0;

    automata_node *Start = PushStruct(Arena, automata_node);
    ZeroStruct(*Start);
    SetFlags(Start, NodeFlag_Start);

    automata_node *LastNode = Start;
    while(Word[LetterIndex] != '\0')
    {
        automata_node *NewNode = PushStruct(Arena, automata_node);
        ZeroStruct(*NewNode);

        BuildTransition(Arena, LastNode, NewNode, Word[LetterIndex++]);
        LastNode = NewNode;
    }
    SetFlags(LastNode, NodeFlag_Accept);

    return Start;
}

internal automata_node *
GetTransitionNode(automata_node *Node, char Input)
{
    automata_node *Result = 0;
    
    for(automata_transition *Transition = Node->Transitions;
        Transition;
        Transition = Transition->Next)
    {
        if(Transition->Input == Input)
        {
            Result = Transition->TransitionNode;
            break;
        }
    }

    return Result;
}

internal node_list *
GetTransitionNodeList(memory_arena *Arena, automata_node *Node, char Input)
{
    node_list *Result = 0;
    
    for(automata_transition *Transition = Node->Transitions;
        Transition;
        Transition = Transition->Next)
    {
        if(Transition->Input == Input)
        {
            if(Result)
            {
                node_list *Temp = Result;
                Result = PushStruct(Arena, node_list);
                ZeroStruct(*Result);

                Result->Node = Transition->TransitionNode;
                Result->Next = Temp;
            }
            else
            {
                Result = PushStruct(Arena, node_list);
                ZeroStruct(*Result);

                Result->Node = Transition->TransitionNode;
            }
        }
    }

    return Result;
}

internal bool32
IsNodeInList(node_list *List, automata_node *Node)
{
    bool32 Result = false;

    node_list *ListIterator = List;
    while(ListIterator)
    {
        if(Node == ListIterator->Node)
        {
            Result = true;
            break;
        }
        ListIterator = ListIterator->Next;
    }
    
    return Result;
}

internal bool32
ListCompare(node_list *A, node_list *B)
{
    bool32 Result = true;

    node_list *IteratorA = A;
    while(IteratorA && Result)
    {
        automata_node *Node = IteratorA->Node;
        bool32 NodeIsInBothLists = false;
        node_list *IteratorB = B;
        while(IteratorB)
        {
            if(Node == IteratorB->Node)
            {
                NodeIsInBothLists = true;
                break;
            }
            IteratorB = IteratorB->Next;
        }
        Result = (Result && NodeIsInBothLists);
        IteratorA = IteratorA->Next;
    }

    node_list *IteratorB = B;
    while(IteratorB && Result)
    {
        automata_node *Node = IteratorB->Node;
        bool32 NodeIsInBothLists = false;
        node_list *IteratorA = A;
        while(IteratorA)
        {
            if(Node == IteratorA->Node)
            {
                NodeIsInBothLists = true;
                break;
            }
            IteratorA = IteratorA->Next;
        }
        Result = (Result && NodeIsInBothLists);
        IteratorB = IteratorB->Next;
    }

    return Result;
}

internal node_list *
EpsilonClosure(memory_arena *Arena, automata_node *A)
{
    node_list *Result = PushStruct(Arena, node_list);
    ZeroStruct(*Result);

    Result->Node = A;

    automata_node *NodeQueue[1024] = {A};
    uint32 QueueIndex = 0;
    uint32 QueueSize = 1;

    while(QueueSize > 0)
    {
        automata_node *Node = NodeQueue[QueueIndex];
        for(automata_transition *Transition = Node->Transitions;
            Transition;
            Transition = Transition->Next)
        {
            automata_node *TestNode = Transition->TransitionNode;
            if((Transition->Input == EPSILON) &&
               (!IsNodeInList(Result, TestNode)))
            {
                if(Result->Node)
                {
                    node_list *Temp = Result;
                    Result = PushStruct(Arena, node_list);
                    Result->Node = TestNode;
                    Result->Next = Temp;
                }
                else
                {
                    Result->Node = TestNode;
                }
                NodeQueue[QueueIndex + QueueSize++] = TestNode;
            }
        }

        QueueIndex++;
        QueueSize--;
    }
    
    return Result;
}

internal node_list *
Concat(node_list *A, node_list *B)
{
    node_list *Result = A;
    
    if(!Result)
    {
        Result = B;
    }
    else
    {    
        node_list *Iterator;
        for(Iterator = A;
            Iterator->Next;
            Iterator = Iterator->Next)
        {
        }
        Iterator->Next = B;
    }
    
    return Result;
}

internal node_list *
RemoveDuplicates(node_list *List)
{
    node_list *Result = List;
    if(List)
    {
        node_list *Iterator = List;
        node_list *LastIterator = 0;
        while(Iterator)
        {
            if(IsNodeInList(Iterator->Next, Iterator->Node))
            {
                if(LastIterator)
                {
                    // free(LastIterator->Next);
                    LastIterator->Next = Iterator->Next;
                    Iterator = Iterator->Next;
                }
                else
                {
                    Result = Result->Next;
                    Iterator = Iterator->Next;
                }
            }
            else
            {
                LastIterator = Iterator;
                Iterator = Iterator->Next;
            }
        }
    }

    return Result;
}

internal automata_node *
ConvertNFAToDFA(memory_arena *Arena, automata_node *Start)
{
    node_list *StartClosure = EpsilonClosure(Arena, Start);

    automata_node *DFAStart = PushStruct(Arena, automata_node);
    ZeroStruct(*DFAStart);

    DFAStart->NFAList = StartClosure;
    SetFlags(DFAStart, NodeFlag_Start);

    for(node_list *Iterator = DFAStart->NFAList;
        Iterator;
        Iterator = Iterator->Next)
    {
        if(IsSet(Iterator->Node, NodeFlag_Accept))
        {
            SetFlags(DFAStart, NodeFlag_Accept);
            break;
        }
    }
        
    automata_node *Queue[1024] = {DFAStart};
    uint32 QueueIndex = 0;
    uint32 QueueSize = 1;

    while(QueueSize > 0)
    {
        for(uint8 Input = 0;
            Input < 127;
            ++Input)
        {
            node_list *List = 0;

            automata_node *CurrentNode = Queue[QueueIndex];
            for(node_list * NFAList= CurrentNode->NFAList;
                NFAList;
                NFAList = NFAList->Next)
            {
                automata_node *NFANode = NFAList->Node;
                node_list *Transitions = GetTransitionNodeList(Arena, NFANode, Input);
                for (node_list *Transition = Transitions;
                     Transition;
                     Transition = Transition->Next)
                {
                    node_list *Closure = EpsilonClosure(Arena, Transition->Node);
                    List = Concat(List, Closure);
                    List = RemoveDuplicates(List);
                }
            }

            if(List)
            {
                automata_node *CheckNode = 0;
                for(uint32 Index = 0;
                    Index < QueueIndex+QueueSize;
                    ++Index)
                {
                    if(ListCompare(Queue[Index]->NFAList, List))
                    {
                        CheckNode = Queue[Index];
                        break;
                    }
                }
            
                if(CheckNode)
                {
                    BuildTransition(Arena, CurrentNode, CheckNode, Input);
                }
                else
                {
                    automata_node *NewNode = PushStruct(Arena, automata_node);
                    ZeroStruct(*NewNode);
                
                    NewNode->NFAList = List;
                    BuildTransition(Arena, CurrentNode, NewNode, Input);

                    for(node_list *Iterator = List;
                        Iterator;
                        Iterator = Iterator->Next)
                    {
                        if(IsSet(Iterator->Node, NodeFlag_Accept))
                        {
                            SetFlags(NewNode, NodeFlag_Accept);
                            break;
                        }
                    }
                
                    Queue[QueueIndex + QueueSize++] = NewNode;
                }
            }
        }
        QueueIndex++;
        QueueSize--;
    }

    return DFAStart;
}

internal automata_node *
SingleCharNFA(memory_arena *Arena, char Char)
{
    automata_node *Result = PushStruct(Arena, automata_node);
    automata_node *End = PushStruct(Arena, automata_node);
    ZeroStruct(*Result);
    ZeroStruct(*End);
    
    SetFlags(Result, NodeFlag_Start);
    SetFlags(End, NodeFlag_Accept);

    BuildTransition(Arena, Result, End, Char);

    return Result;
}

internal automata_node *
ManyCharNFA(memory_arena *Arena, char *Char, uint32 Amount)
{
    automata_node *Result = PushStruct(Arena, automata_node);
    automata_node *End = PushStruct(Arena, automata_node);
    ZeroStruct(*Result);
    ZeroStruct(*End);
    
    SetFlags(Result, NodeFlag_Start);
    SetFlags(End, NodeFlag_Accept);

    for(uint32 CharIndex = 0;
        CharIndex < Amount;
        ++CharIndex)
    {
        BuildTransition(Arena, Result, End, Char[CharIndex]);
    }

    return Result;   
}

internal automata_node *
FindEndNode(automata_node *Start)
{
    // NOTE: this will return only the first end node it finds.

    automata_node *Result = 0;

    automata_node *Queue[1024] = {Start};
    uint32 QueueSize = 1;
    uint32 QueueIndex = 0;

    while(QueueSize > 0)
    {
        automata_node *CurrentNode = Queue[QueueIndex];
        
        for(automata_transition *Transition = CurrentNode->Transitions;
            Transition;
            Transition = Transition->Next)
        {
            automata_node *TestNode  = Transition->TransitionNode;
            
            if(IsSet(TestNode, NodeFlag_Accept))
            {
                Result = TestNode;
                QueueSize = 1; // double break
                break;
            }
            else
            {
                bool32 NodeTouched = false;
                for(uint32 Index = 0;
                    Index < QueueIndex+QueueSize;
                    ++Index)
                {
                    if(Queue[Index] == TestNode)
                    {
                        NodeTouched = true;
                        break;
                    }
                }

                if(!NodeTouched)
                {
                    Queue[QueueIndex + QueueSize++] = TestNode;
                }
            }
        }

        QueueIndex++;
        QueueSize--;
    }

    return Result;
}

internal automata_node *
DeepCopy(memory_arena *Arena, automata_node *Start)
{

    automata_node *Result = PushStruct(Arena, automata_node);
    ZeroStruct(*Result);
    SetFlags(Result, Start->Flags);

    node_pair StartPair = {};
    StartPair.Original = Start;
    StartPair.Copy = Result;
    
    node_pair Queue[1024] = {StartPair};
    uint32 QueueSize = 1;
    uint32 QueueIndex = 0;

    while(QueueSize > 0)
    {
        node_pair CurrentPair = Queue[QueueIndex];
        
        for(automata_transition *Transition = CurrentPair.Original->Transitions;
            Transition;
            Transition = Transition->Next)
        {
            automata_node *TestNode = Transition->TransitionNode;

            bool32 Touched = false;
            automata_node *NextCopiedNode = 0;
            for(uint32 Index = 0;
                Index < QueueIndex+QueueSize;
                ++Index)
            {
                if(Queue[Index].Original == TestNode)
                {
                    Touched = true;
                    NextCopiedNode = Queue[Index].Copy;
                    break;
                }
            }

            if(!Touched)
            {
                NextCopiedNode = PushStruct(Arena, automata_node);
                ZeroStruct(*NextCopiedNode);

                SetFlags(NextCopiedNode, TestNode->Flags);
                Queue[QueueIndex + QueueSize].Original = TestNode;
                Queue[QueueIndex + QueueSize++].Copy = NextCopiedNode;
            }

            BuildTransition(Arena, CurrentPair.Copy, NextCopiedNode, Transition->Input);
        }

        QueueIndex++;
        QueueSize--;
    }

    return Result;
}

internal automata_node *
ConcatNFA(memory_arena *Arena, automata_node *A, automata_node *B)
{
    A = DeepCopy(Arena, A);
    B = DeepCopy(Arena, B);

    automata_node *Result = A;
    ClearFlags(B, NodeFlag_Start);

    automata_node *EndOfA = FindEndNode(A);
    ClearFlags(EndOfA, NodeFlag_Accept);

    BuildTransition(Arena, EndOfA, B, EPSILON);

    return Result;
}

internal automata_node *
OrNFA(memory_arena *Arena, automata_node *A, automata_node *B)
{
    A = DeepCopy(Arena, A);
    B = DeepCopy(Arena, B);
    
    automata_node *Result = PushStruct(Arena, automata_node);
    automata_node *End = PushStruct(Arena, automata_node);
    ZeroStruct(*Result);
    ZeroStruct(*End);
    SetFlags(Result, NodeFlag_Start);
    SetFlags(End, NodeFlag_Accept);
    
    ClearFlags(A, NodeFlag_Start);
    ClearFlags(B, NodeFlag_Start);
    automata_node *EndOfA = FindEndNode(A);
    ClearFlags(EndOfA, NodeFlag_Accept);
    automata_node *EndOfB = FindEndNode(B);
    ClearFlags(EndOfB, NodeFlag_Accept);

    BuildTransition(Arena, Result, A, EPSILON);
    BuildTransition(Arena, Result, B, EPSILON);
    BuildTransition(Arena, EndOfA, End, EPSILON);
    BuildTransition(Arena, EndOfB, End, EPSILON);

    return Result;
}

internal automata_node *
StarNFA(memory_arena *Arena, automata_node *A)
{
    A = DeepCopy(Arena, A);
    
    automata_node *Result = PushStruct(Arena, automata_node);
    automata_node *End = PushStruct(Arena, automata_node);
    ZeroStruct(*Result);
    ZeroStruct(*End);
    SetFlags(Result, NodeFlag_Start);
    SetFlags(End, NodeFlag_Accept);
    
    ClearFlags(A, NodeFlag_Start);
    automata_node *EndOfA = FindEndNode(A);
    ClearFlags(EndOfA, NodeFlag_Accept);

    BuildTransition(Arena, Result, A, EPSILON);
    BuildTransition(Arena, Result, End, EPSILON);
    BuildTransition(Arena, EndOfA, End, EPSILON);
    BuildTransition(Arena, EndOfA, Result, EPSILON);

    return Result;
}

internal automata_node *
GenerateDFAForWords(memory_arena *Arena, char **Words, uint32 TotalWords)
{
    automata_node *Start = PushStruct(Arena, automata_node);
    ZeroStruct(*Start);
    SetFlags(Start, NodeFlag_Start);

    for(uint32 WordIndex = 0;
        WordIndex < TotalWords;
        ++WordIndex)
    {
        char *Word = Words[WordIndex];

        uint32 LetterIndex = 0;
        automata_node *LastNode = Start;
        while(Word[LetterIndex] != '\0')
        {
            automata_node *NewNode = GetTransitionNode(LastNode, Word[LetterIndex]);

            if(NewNode)
            {
                LetterIndex++;
            }
            else
            {
                NewNode = PushStruct(Arena, automata_node);
                ZeroStruct(*NewNode);
                BuildTransition(Arena, LastNode, NewNode, Word[LetterIndex++]);
            }
            LastNode = NewNode;
        }
        SetFlags(LastNode, NodeFlag_Accept);
    }

    return Start;    
}
