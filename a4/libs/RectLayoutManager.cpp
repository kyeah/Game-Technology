#include "RectLayoutManager.h"

RectLayoutManager::RectList::iterator RectLayoutManager::addDataBelow(RectLayoutManager::Rect &Data)
{
  bool MoveIt = false;
  bool FoundIt = false;
  RectList::iterator itStart;
  RectList::iterator itLastChecked;
  RectList::iterator itCurrent;
  RectList::iterator itInsert;

  unsigned short depth = 0;
  RectLayoutManager::Rect &r = Data;
  short height = r.getBottom() - r.getTop();
  if (height > mMaxRectHeight)
    mMaxRectHeight = height;

  // find the first RECT  that has .bottom > r.top
  // first possible intersect
  r.dy -= height;
  itStart = lower_bound(mList.begin(), mList.end(), r, &_fLessBottom);
  r.dy += height;

  // it's safe to add it at the back of the list
  if (itStart == mList.end())
    {
      mList.push_back(r);
      return --(mList.end());
    }

  // insert it temporarily (we will move it at the right place, afterwords)
  itInsert = mList.insert(itStart,r);

  for (itCurrent = itStart, itLastChecked = itInsert;itCurrent != mList.end();itCurrent++)
    {
      // Can't intersect r so i skip it
      if ((*itCurrent).getRight() < r.getLeft())
        continue;

      // Can't intersect r so i skip it
      if (r.getRight() < (*itCurrent).getLeft())
        continue;

      // Can't intersect r so i skip it
      if (r.getTop() > (*itCurrent).getBottom())
        continue;

      short diff = (*itCurrent).getTop() - (*itLastChecked).getBottom();
      short diff2 = mMaxRectHeight - ((*itCurrent).getBottom() - (*itCurrent).getTop());
      if (diff > 0) // above the last checked
        {
          // If no rect overlapped r, then there is no need to move it
          if (!MoveIt && (diff > diff2))
            {
              FoundIt = true;
              itLastChecked = itStart;
              break;
            }
          else
            MoveIt = true;

          if (mDepth && (depth >= mDepth))
            break;

          // This is above r, so i check if its enought space to move r
          if (diff > height + diff2 + 2*mMinDistance)
            {
              r.dy = ((*itLastChecked).getBottom() + mMinDistance + 1) - r.getTop();
              FoundIt = true;
              break;
            }

          depth++;
        }
      else // it overlaps
        MoveIt = true;

      itLastChecked = itCurrent;
    }

  if (itCurrent == mList.end())
    {
      if (MoveIt)
        r.dy = ((*itLastChecked).getBottom() + mMinDistance + 1) - r.getTop();
      else
        itLastChecked = itStart;

      FoundIt = true;
    }

  mList.erase(itInsert);

  if (FoundIt)
    {
      if (r.getBottom() > mBoundBottom)
        return mList.end(); // out of bounds

      itInsert = lower_bound(itLastChecked, itCurrent, r);
      itInsert = mList.insert(itInsert,r);

      return itInsert;
    };

  return mList.end();
}

RectLayoutManager::RectList::iterator RectLayoutManager::addDataAbove(RectLayoutManager::Rect &Data)
{
  bool MoveIt = false;
  bool FoundIt = false;
  RectList::iterator itStart;
  RectList::iterator itLastChecked;
  RectList::iterator itCurrent;
  RectList::iterator itInsert;
 
  unsigned short depth = 0;
  RectLayoutManager::Rect &r = Data;
  short height = r.getBottom() - r.getTop();
 
  if (height > mMaxRectHeight)
    mMaxRectHeight = height;
 
  // find the first RECT  that has .bottom > r.top
  // first possible intersect
  r.dy += height;
  itStart = lower_bound(mList.begin(), mList.end(), r, &_fGreaterTop);
  r.dy -= height;
 
  // it's safe to add it at the back of the list
  if (itStart == mList.end())
    {
      mList.push_back(r);
      return --(mList.end());
    }
 
  // insert it temporarily (we will move it at the right place, afterwords)
  itInsert = mList.insert(itStart,r);
 
  for (itCurrent = itStart, itLastChecked = itInsert;itCurrent != mList.end();itCurrent++)
    {
      // Can't intersect r so i skip it
      if ((*itCurrent).getRight() < r.getLeft())
        continue;
 
      // Can't intersect r so i skip it
      if (r.getRight() < (*itCurrent).getLeft())
        continue;
 
      // Can't intersect r so i skip it
      if (r.getBottom() < (*itCurrent).getTop())
        continue;
 
      short diff = (*itLastChecked).getTop() - (*itCurrent).getBottom();
      short diff2 = mMaxRectHeight - ((*itCurrent).getBottom() - (*itCurrent).getTop());
      if (diff > 0) // above the last checked
        {
          // If no rect overlapped r, then there is no need to move it
          if (!MoveIt && (diff > diff2))
            {
              FoundIt = true;
              itLastChecked = itStart;
              break;
            }
          else
            MoveIt = true;
 
          if (mDepth && (depth >= mDepth))
            break;
 
          // This is above r, so i check if its enought space to move r
          if (diff > height + diff2 + 2*mMinDistance)
            {
              r.dy = -(r.getBottom() - ((*itLastChecked).getTop() - mMinDistance - 1));
              FoundIt = true;
              break;
            }
 
          depth++;
        }
      else // it overlaps
        MoveIt = true;
 
      itLastChecked = itCurrent;
    }
 
  if (itCurrent == mList.end())
    {
      if (MoveIt)
        r.dy = -(r.getBottom() - ((*itLastChecked).getTop() - mMinDistance - 1));
      else
        itLastChecked = itStart;
 
 
      FoundIt = true;
    }
 
  mList.erase(itInsert);
 
  if (FoundIt)
    {
      if (r.getTop() < mBoundTop)
        return mList.end(); // out of bounds
 
      itInsert = lower_bound(itLastChecked, itCurrent, r, _fGreaterTop);
      itInsert = mList.insert(itInsert,r);
 
      return itInsert;
    };
 
  return mList.end();
}
