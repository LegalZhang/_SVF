
#include "SABER/FileChecker.h"

using namespace SVF;
using namespace SVFUtil;


void FileChecker::reportBug(ProgSlice* slice)
{

    if(isAllPathReachable() == false && isSomePathReachable() == false)
    {
        // full leakage
        GenericBug::EventStack eventStack = { SVFBugEvent(SVFBugEvent::SourceInst, getSrcCSID(slice->getSource())) };
        report.addSaberBug(GenericBug::FILENEVERCLOSE, eventStack);
    }
    else if (isAllPathReachable() == false && isSomePathReachable() == true)
    {
        GenericBug::EventStack eventStack;
        slice->evalFinalCond2Event(eventStack);
        eventStack.push_back(
            SVFBugEvent(SVFBugEvent::SourceInst, getSrcCSID(slice->getSource())));
        report.addSaberBug(GenericBug::FILEPARTIALCLOSE, eventStack);
    }
}
