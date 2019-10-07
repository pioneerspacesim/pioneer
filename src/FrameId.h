#ifndef FRAMEID_H_INCLUDED
#define FRAMEID_H_INCLUDED

typedef int FrameId;

constexpr FrameId noFrameId = -1;

extern bool IsIdValid(FrameId fId);

#endif // FRAMEID_H_INCLUDED
