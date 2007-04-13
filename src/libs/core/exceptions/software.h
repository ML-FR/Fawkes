
/***************************************************************************
 *  software.h - basic software exceptions
 *
 *  Generated: Wed Oct 04 18:37:35 2006
 *  Copyright  2006  Tim Niemueller [www.niemueller.de]
 *
 *  $Id$
 *
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307, USA.
 */

#ifndef __CORE_EXCEPTIONS_SOFTWARE_H_
#define __CORE_EXCEPTIONS_SOFTWARE_H_

#include <core/exception.h>

class NullPointerException : public Exception {
 public:
  NullPointerException(const char *msg);
};


class DivisionByZeroException : public Exception {
 public:
  DivisionByZeroException(const char *msg);
};


class TypeMismatchException : public Exception {
 public:
  TypeMismatchException(const char *msg);
};


class DestructionInProgressException : public Exception {
 public:
  DestructionInProgressException(const char *msg);
};


class NotLockedException : public Exception {
 public:
  NotLockedException(const char *msg);
};


class NonPointerTypeExpectedException : public Exception {
 public:
  NonPointerTypeExpectedException(const char *msg);
};

class MissingParameterException : public Exception {
 public:
  MissingParameterException(const char *msg);
};

class IllegalArgumentException : public Exception {
 public:
  IllegalArgumentException(const char *msg);
};

#endif
