/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jesse Calvert $
   ======================================================================== */

#include "rejex.h"
#include "finite_automata.cpp"

internal void
CheckQuantifiers(memory_arena *Arena, automata_node **NFA, char **CharPtr)
{
    if(*(*CharPtr + 1) == '*')
    {
        *NFA = StarNFA(Arena, *NFA);
        ++*CharPtr;
    }
    else if(*(*CharPtr + 1) == '+')
    {
        *NFA = ConcatNFA(Arena, *NFA, (StarNFA(Arena, *NFA)));
        ++*CharPtr;
    }
    else if(*(*CharPtr + 1) == '?')
    {
        *NFA = OrNFA(Arena, *NFA, SingleCharNFA(Arena, EPSILON));
        ++*CharPtr;
    }
}

internal automata_node *
GenerateNFA(memory_arena *Arena, char *Regex)
{
    automata_node *Start = SingleCharNFA(Arena, EPSILON);

    char *CharPtr = Regex;
    while(*CharPtr != 0)
    {
        switch(*CharPtr)
        {
            case '(':
            {
                char SubRegex[128] = {};

                bool32 FoundCloseParen = false;
                uint32 Index;
                for(Index = 0;
                    !FoundCloseParen;
                    ++Index)
                {
                    if(CharPtr[Index+1] == ')')
                    {
                        FoundCloseParen = true;
                    }
                    else
                    {
                        SubRegex[Index] = CharPtr[Index+1];                        
                    }
                }
                CharPtr += Index;

                automata_node *NextNFA = GenerateNFA(Arena, SubRegex);
                CheckQuantifiers(Arena, &NextNFA, &CharPtr);
                
                Start = ConcatNFA(Arena, Start, NextNFA);
                ++CharPtr;

            } break;

            case '[':
            {
                char CharOrList[128] = {};
                ++CharPtr;
                uint32 Index = 0;
                uint32 CharListIndex = 0;
                while(CharPtr[Index] != ']')
                {
                    if(CharPtr[Index] == '-')
                    {
                        if(IsDigit(CharPtr[Index - 1]))
                        {
                            uint32 StartIndex = GetDigitIndex(CharPtr[Index - 1]) + 1;
                            uint32 EndIndex = GetDigitIndex(CharPtr[Index + 1]);

                            Assert(StartIndex < EndIndex);

                            for(uint32 DigitIndex = StartIndex;
                                DigitIndex <= EndIndex;
                                ++DigitIndex)
                            {
                                CharOrList[CharListIndex + (DigitIndex - StartIndex)] = Digits[DigitIndex];
                            }

                            CharListIndex += EndIndex - StartIndex + 1;
                            Index += 2;
                        }
                        else
                        {
                            uint32 StartIndex = GetLetterIndex(CharPtr[Index - 1]) + 1;
                            uint32 EndIndex = GetLetterIndex(CharPtr[Index + 1]);

                            Assert(StartIndex < EndIndex);

                            for(uint32 LetterIndex = StartIndex;
                                LetterIndex <= EndIndex;
                                ++LetterIndex)
                            {
                                CharOrList[CharListIndex + (LetterIndex - StartIndex)] = Letters[LetterIndex];
                            }

                            CharListIndex += EndIndex - StartIndex + 1;
                            Index += 2;
                        }
                    }
                    else
                    {
                        CharOrList[CharListIndex++] = CharPtr[Index++];
                    }
                }
                CharPtr += Index;

                automata_node *NextNFA = ManyCharNFA(Arena, CharOrList, CharListIndex);
                CheckQuantifiers(Arena, &NextNFA, &CharPtr);
                
                Start = ConcatNFA(Arena, Start, NextNFA);
                ++CharPtr;
            } break;

            default:
            {
                automata_node *NextNFA = SingleCharNFA(Arena, *CharPtr);
                CheckQuantifiers(Arena, &NextNFA, &CharPtr);

                Start = ConcatNFA(Arena, Start, NextNFA);
                ++CharPtr;
            } break;
        }
    }

    return Start;
}

internal regex
CompileRegex(memory_arena *PermanentArena, char *Regex)
{
    regex Result = {};

    uint32 RegexLength = 0;
    while(Regex[RegexLength] != 0)
    {
        RegexLength++;
    }
    SetString(Result.String, Regex, RegexLength);

    memory_index TotalMemory = Megabytes(32);
    void * AllMemory = calloc(1, TotalMemory);
    memory_arena TemporaryArena = {};
    InitializeArena(&TemporaryArena, TotalMemory, (uint8 *)AllMemory);

    temporary_memory TempMem = BeginTemporaryMemory(&TemporaryArena);

    automata_node *Start = GenerateNFA(&TemporaryArena, Regex);
    Result.Table = ConvertToDFATable(PermanentArena, ConvertNFAToDFA(&TemporaryArena, Start));

    EndTemporaryMemory(TempMem);

    return Result;
}

internal bool32
Match(regex *Regex, char *Input)
{
    bool32 Result = RunFiniteAutomata(&Regex->Table, Input);
    return Result;
}

internal bool32
Match(char *RegexString, char *Input)
{
    memory_index TotalMemory = Megabytes(32);
    void * AllMemory = calloc(1, TotalMemory);
    memory_arena TemporaryArena = {};
    InitializeArena(&TemporaryArena, TotalMemory, (uint8 *)AllMemory);

    temporary_memory TempMem = BeginTemporaryMemory(&TemporaryArena);

    regex Regex = CompileRegex(&TemporaryArena, RegexString);
    bool32 Result = RunFiniteAutomata(&Regex.Table, Input);

    EndTemporaryMemory(TempMem);

    return Result;
}

int32
main(int32 ArgC, char **ArgV)
{
    memory_index TotalMemory = Megabytes(32);
    void * AllMemory = calloc(1, TotalMemory);
    
    memory_arena PermanentArena = {};
    InitializeArena(&PermanentArena, TotalMemory, AllMemory);
    
    char *DigitRegex = "[0-9]";
    regex Digit = CompileRegex(&PermanentArena, DigitRegex);

    char *LetterRegex = "[a-zA-Z]";
    regex Letter = CompileRegex(&PermanentArena, LetterRegex);
    
    char *NumberRegex = "[0-9]*";
    regex Number = CompileRegex(&PermanentArena, NumberRegex);

    char *IdentiferRegex = "[a-zA-Z][0-9a-zA-Z]*";
    regex Identifier = CompileRegex(&PermanentArena, IdentiferRegex);

    char *TestRegex = "([abc]t*)+123";
    regex Test = CompileRegex(&PermanentArena, TestRegex);

    char *TestStrings[] = {"9", "33", "word", "z", "837a", "fjie877f", "y2", "yu2", "btttcta123"};
    regex *Regexes[] = {&Digit, &Letter, &Number, &Identifier, &Test};

    for(uint32 RegexIndex = 0;
        RegexIndex < ArrayCount(Regexes);
        ++RegexIndex)
    {
        cout << Regexes[RegexIndex]->String << endl;
        for(uint32 TestIndex = 0;
            TestIndex < ArrayCount(TestStrings);
            ++TestIndex)
        {
            cout << TestStrings[TestIndex] << "\t" <<  Match(Regexes[RegexIndex], TestStrings[TestIndex]) << endl;
        }
        cout << endl;
    }

    cout << Match("(123)*Z", "123123123123Z") << endl;

    char Done[8];
    cin >> Done;
}
