/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "Util.h"
#include "DVDTSCorrection.h"
#include "DVDClock.h"
#include "utils/log.h"

#define MAXERR 0.01

using namespace std;

CPullupCorrection::CPullupCorrection()
{
  Flush();
}

void CPullupCorrection::Flush()
{
  m_ringpos = 0;
  m_ptscorrection = 0.0;
  m_prevpts = DVD_NOPTS_VALUE;
  m_patternpos = 0;
  m_ringfill = 0;
  m_pattern.clear();
  m_haspattern = false;
  m_patternlength = 0;
}

void CPullupCorrection::Add(double pts)
{
  //can't get a diff with just one pts
  if (m_prevpts == DVD_NOPTS_VALUE)
  {
    m_prevpts = pts;
    return;
  }
  
  //increase the ringbuffer position
  m_ringpos = (m_ringpos + 1) % DIFFRINGSIZE;
  //add the current diff to the ringbuffer
  m_diffring[m_ringpos] = pts - m_prevpts;
  //save the pts
  m_prevpts = pts;
  
  //only search for patterns in a full ringbuffer
  m_ringfill++;
  if (m_ringfill < DIFFRINGSIZE)
    return;
  
  m_ringfill = DIFFRINGSIZE;
  
  //get the current pattern in the ringbuffer
  vector<double> pattern;
  GetPattern(pattern);
  
  //check if the pattern is the same as the saved pattern
  //and if it is actually a pattern
  if (!CheckPattern(pattern))
  {
    m_ptscorrection = 0.0; //no pattern no correction
    if (m_haspattern)
    {
      m_haspattern = false;
      m_patternlength = 0;
      CLog::Log(LOGDEBUG, "CPullupCorrection: pattern lost");
    }
    return;
  }
  else if (!m_haspattern)
  {
    m_haspattern = true;
    m_patternlength = m_pattern.size();
    CLog::Log(LOGDEBUG, "CPullupCorrection: detected pattern of length %zu: %s",
              pattern.size(), GetPatternStr().c_str());
  }
  
  //calculate where we are in the pattern
  double ptsinpattern = 0.0;
  for (int i = 0; i < m_patternpos; i++)
  {
    ptsinpattern += m_pattern[m_pattern.size() - i - 1];
  }
  
  double frameduration = CalcFrameDuration();
  
  //correct the last pts based on where we should be according to the frame duration
  m_ptscorrection = (frameduration * m_patternpos) - ptsinpattern;
}

//gets a diff diffnr into the past
inline double CPullupCorrection::GetDiff(int diffnr)
{
  //m_ringpos is the last added diff, so if we want to go in the past we have to move back in the ringbuffer
  int pos = m_ringpos - diffnr;
  if (pos < 0) pos += DIFFRINGSIZE;
  
  return m_diffring[pos];
}

//calculate the current pattern in the ringbuffer
void CPullupCorrection::GetPattern(std::vector<double>& pattern)
{
  int difftypesbuff[DIFFRINGSIZE]; //difftypes of the diffs, difftypesbuff[0] is the last added diff,
                                   //difftypesbuff[1] the one added before that etc
  
  //get the difftypes
  vector<double> difftypes;
  GetDifftypes(difftypes);
  
  //mark each diff with what difftype it is
  for (int i = 0; i < DIFFRINGSIZE; i++)
  {
    for (unsigned int j = 0; j < difftypes.size(); j++)
    {
      if (MatchDiff(GetDiff(i), difftypes[j]))
      {
        difftypesbuff[i] = j;
        break;
      }
    }
  }
  
  //we check for patterns to the length of DIFFRINGSIZE / 3
  for (int i = 1; i <= DIFFRINGSIZE / 3; i++)
  {
    bool hasmatch = true;
    for (int j = 1; j <= DIFFRINGSIZE / i; j++)
    {
      int nrdiffs = i;
      //we want to check the full buffer to see if the pattern repeats
      //but we can't go beyond the buffer
      if (j * i + i > DIFFRINGSIZE)
        nrdiffs = DIFFRINGSIZE - j * i;
      
      if (nrdiffs < 1)  //if the buffersize can be cleanly divided by i we're done here
        break;
      
      if (!MatchDifftype(difftypesbuff, difftypesbuff + j * i, nrdiffs))
      {
        hasmatch = false;
        break;
      }
    }
    
    if (hasmatch)
    {
      BuildPattern(pattern, i);
      break;
    }
  }
}

//calculate the different types of diffs we have
void CPullupCorrection::GetDifftypes(vector<double>& difftypes)
{
  for (int i = 0; i < DIFFRINGSIZE; i++)
  {
    bool hasmatch = false;
    for (unsigned int j = 0; j < difftypes.size(); j++)
    {
      if (MatchDiff(GetDiff(i), difftypes[j]))
      {
        hasmatch = true;
        break;
      }
    }
    
    //if we don't have a match with a saved difftype, we add it as a new one
    if (!hasmatch)
      difftypes.push_back(GetDiff(i));
  }
}

//builds a pattern of timestamps in the ringbuffer
void CPullupCorrection::BuildPattern(std::vector<double>& pattern, int patternlength)
{
  for (int i = 0; i < patternlength; i++)
  {
    double avgdiff = 0.0;
    for (int j = 0; j < DIFFRINGSIZE / patternlength; j++)
    {
      avgdiff += GetDiff(j * patternlength + i);
    }
    avgdiff /= DIFFRINGSIZE / patternlength;
    pattern.push_back(avgdiff);
  }
}

inline bool CPullupCorrection::MatchDiff(double diff1, double diff2)
{
  if (fabs(diff1) < MAXERR && fabs(diff2) < MAXERR)
    return true; //very close to 0.0
  
  if (diff2 == 0.0)
    return false; //don't want to divide by 0
  
  return fabs(1.0 - (diff1 / diff2)) <= MAXERR;
}

//check if diffs1 is the same as diffs2
inline bool CPullupCorrection::MatchDifftype(int* diffs1, int* diffs2, int nrdiffs)
{
  for (int i = 0; i < nrdiffs; i++)
  {
    if (diffs1[i] != diffs2[i])
      return false;
  }
  return true;
}

//check if our current detected pattern is the same as the one we saved
bool CPullupCorrection::CheckPattern(std::vector<double>& pattern)
{
  //if no pattern was detected or if the size of the patterns differ we don't have a match
  if (pattern.size() != m_pattern.size() || pattern.size() < 1 || (pattern.size() == 1 && pattern[0] <= MAXERR))
  {
    m_pattern = pattern; //save the current pattern
    m_patternpos = 0;    //reset the position
    return false;
  }
  
  //the saved pattern should have moved 1 diff into the past
  m_patternpos = (m_patternpos + 1) % m_pattern.size();
  
  //check if the current pattern matches the saved pattern, with an offset of 1
  for (unsigned int i = 0; i < m_pattern.size(); i++)
  {
    double diff = pattern[(m_patternpos + i) % pattern.size()];
    
    if (!MatchDiff(diff, m_pattern[i]))
    {
      m_pattern = pattern; //save the current pattern
      m_patternpos = 0;    //reset the position
      return false;
    }
    //we save the pattern, in case it changes very slowly
    m_pattern[i] = diff;
  }
  
  return true;
}

//calculate how long each frame should last from the saved pattern
double CPullupCorrection::CalcFrameDuration()
{
  double frameduration = 0.0;
  
  if (m_haspattern)
  {
  for (unsigned int i = 0; i < m_pattern.size(); i++)
  {
    frameduration += m_pattern[i];
  }
  return frameduration / m_pattern.size();
}

  return DVD_NOPTS_VALUE;
}

//looks pretty in the log
CStdString CPullupCorrection::GetPatternStr()
{
  CStdString patternstr;
  
  for (unsigned int i = 0; i < m_pattern.size(); i++)
  {
    patternstr.AppendFormat("%.2f ", m_pattern[i]);
  }
  
  return patternstr;
}
