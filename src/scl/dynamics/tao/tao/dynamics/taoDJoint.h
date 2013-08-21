/* Copyright (c) 2005 Arachi, Inc. and Stanford University. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/** Edited 2013-08-21 : Samir Menon <smenon@stanford.edu>
 */

#ifndef _taoDJoint_h
#define _taoDJoint_h

#include <string>

#include <tao/matrix/TaoDeTypes.h>

class taoDVar;

/*!
 *	\brief abstract joint class for articulated body
 *	\ingroup taoDynamics
 *
 *	This class should be used as a base class and implemented accordingly.
 *
 *	NOTE TODO Samir > This class is useless! Delete it.
 */
class taoDJoint
{
public:
	virtual ~taoDJoint() {}

	virtual taoDVar* getDVar() = 0;

	virtual deFloat getDamping() = 0;
	virtual deFloat getInertia() = 0;

	virtual void addQdelta() = 0;
	virtual void addDQdelta() = 0;
	virtual void zeroTau() = 0;

	std::string name_;
};

#endif // _taoDJoint_h
