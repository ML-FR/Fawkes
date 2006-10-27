
/***************************************************************************
 *  message_queue.cpp - BlackBoard Interface message queue
 *
 *  Generated: Tue Oct 18 15:43:29 2006
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <interfaces/message_queue.h>
#include <interfaces/message.h>

#include <core/threading/mutex.h>
#include <core/exceptions/software.h>

#include <cstddef>
#include <stdlib.h>

/** @class MessageAlreadyQueuedException interfaces/message_queue.h
 * Message already enqueued exception.
 * This exception is thrown if you try to enqueue a message that has already
 * been enqueued in another message queue. This is an illegal operation. If you
 * need to enqueue a message multiple times use the copy constructor to do this.
 */


/** Constructor. */
MessageAlreadyQueuedException::MessageAlreadyQueuedException()
  : Exception("Message already enqueued in another MessageQueue.")
{
}



/** @class MessageQueue interfaces/message_queue.h
 * Message queue used in interfaces.
 * This message queue handles the basic messaging operations. The methods the
 * Interface provides for handling message queues are forwarded to a
 * MessageQueue instance.
 * @see Interface
 */


/** Constructor */
MessageQueue::MessageQueue()
{
  list = NULL;
  end_el = NULL;
  next_msg_id = 1;
  mutex = new Mutex();
}


/** Destructor */
MessageQueue::~MessageQueue()
{
  flush();
  delete mutex;
}


/** Delete all messages from queue.
 * This method deletes all messages from the queue.
 */
void
MessageQueue::flush()
{
  mutex->lock();
  // free list elements
  msg_list_t *l = list;
  msg_list_t *next;
  while ( l ) {
    next = l->next;
    l->msg->unref();
    free(l);
    l = next;
  }
  mutex->unlock();
}


/** Append message to queue.
 * @param msg Message to append
 * @return message queue id of the appended message.
 */
unsigned int
MessageQueue::append(Message *msg)
{
  if ( msg->message_id != 0 ) {
    throw MessageAlreadyQueuedException();
  }
  mutex->lock();
  unsigned int new_msg_id = 0;
  msg->ref();
  if ( list == NULL ) {
    list = (msg_list_t *)malloc(sizeof(msg_list_t));
    list->next = NULL;
    list->msg = msg;
    list->msg_id = next_msg_id++;
    end_el = list;
    new_msg_id = list->msg_id;
  } else {
    msg_list_t *l = (msg_list_t *)malloc(sizeof(msg_list_t));
    l->next = NULL;
    l->msg = msg;
    l->msg_id = next_msg_id++;
    end_el->next = l;
    end_el = l;
    new_msg_id = l->msg_id;
  }

  mutex->unlock();
  return new_msg_id;
}


/** Enqueue message after given iterator.
 * @param it Iterator
 * @param msg Message to enqueue
 * @return message queue id of the appended message.
 * @exception NullPointerException thrown if iterator is end iterator.
 * @exception NotLockedException thrown if message queue is not locked during this operation.
 */
unsigned int
MessageQueue::insert_after(const MessageIterator &it, Message *msg)
{
  if ( mutex->tryLock() ) {
    mutex->unlock();
    throw NotLockedException("Message queue must be locked to insert messages after iterator.");
  }
  if ( it.cur == NULL ) {
    throw NullPointerException("Cannot append message at end element.");
  }
  if ( msg->message_id != 0 ) {
    throw MessageAlreadyQueuedException();
  }
  msg->ref();
  msg_list_t *l = (msg_list_t *)malloc(sizeof(msg_list_t));
  l->next = it.cur->next;
  l->msg = msg;
  l->msg_id = next_msg_id++;
  it.cur->next = l;
  if ( l->next == NULL ) {
    end_el = l;
  }
  return l->msg_id;
}


/** Remove message from queue.
 * @param msg message to remove
 */
void
MessageQueue::remove(const Message *msg)
{
  mutex->lock();
  msg_list_t *l = list;
  msg_list_t *p = NULL;
  while ( l ) {
    if ( l->msg == msg ) {
      remove(l, p);
      break;
    } else {
      p = l;
      l = l->next;
    }
  }
  mutex->unlock();
}


/** Remove message from queue by message id.
 * @param msg_id id of message to remove
 */
void
MessageQueue::remove(const unsigned int msg_id)
{
  mutex->lock();
  msg_list_t *l = list;
  msg_list_t *p = NULL;
  while ( l ) {
    if ( l->msg_id == msg_id ) {
      remove(l, p);
      break;
    } else {
      p = l;
      l = l->next;
    }
  }
  mutex->unlock();
}


/** Remove message from list.
 * @param l list item to remove
 * @param p predecessor of element, may be NULL if there is none
 */
void
MessageQueue::remove(msg_list_t *l, msg_list_t *p)
{
  if ( mutex->tryLock() ) {
    mutex->unlock();
    throw NotLockedException("Protected remove must be made safe by locking.");
  }
  if ( p ) {
    p->next = l->next;
  } else {
    // was first element
    list = l->next;
  }
  l->msg->unref();
  free(l);
}


/** Get number of messages in queue.
 * @return number of messages in queue.
 */
unsigned int
MessageQueue::size() const
{
  mutex->lock();
  unsigned int rv = 0;
  msg_list_t *l = list;
  while ( l ) {
    ++rv;
    l = l->next;
  }

  mutex->unlock();
  return rv;
}


/** Lock message queue.
 * No operations can be performed on the message queue after locking it.
 * Note that you cannot call any method of the message queue as long as
 * the queue is locked. Use lock() only to have a secure run-through with
 * the MessageIterator.
 */
void
MessageQueue::lock()
{
  mutex->lock();
}


/** Try to lock message queue.
 * No operations can be performed on the message queue after locking it.
 * Note that you cannot call any method of the message queue as long as
 * the queue is locked. Use tryLock() only to have a secure run-through with
 * the MessageIterator.
 * @return true, if the lock has been aquired, false otherwise.
 */
bool
MessageQueue::tryLock()
{
  return mutex->tryLock();
}


/** Unlock message queue.
 */
void
MessageQueue::unlock()
{
  mutex->unlock();
}


/** Get iterator to first element in message queue.
 * @return iterator to first element in message queue
 * @exception NotLockedException thrown if message queue is not locked during this operation.
 */
MessageQueue::MessageIterator
MessageQueue::begin()
{
  if ( mutex->tryLock() ) {
    mutex->unlock();
    throw NotLockedException("Message queue must be locked to get begin iterator.");
  }
  return MessageIterator(list);
}


/** Get iterator to element beyond end of message queue list.
 * @return iterator to element beyond end of message queue list
 * @exception NotLockedException thrown if message queue is not locked during this operation.
 */
MessageQueue::MessageIterator
MessageQueue::end()
{
  if ( mutex->tryLock() ) {
    mutex->unlock();
    throw NotLockedException("Message queue must be locked to get end iterator.");
  }
  return MessageIterator();
}


/** Constructor
 * @param cur Current element for message list
 */
MessageQueue::MessageIterator::MessageIterator(msg_list_t *cur)
{
  this->cur = cur;
}


/** Constructor */
MessageQueue::MessageIterator::MessageIterator()
{
  cur = NULL;
}


/** Copy constructor.
 * @param it Iterator to copy
 */
MessageQueue::MessageIterator::MessageIterator(const MessageIterator &it)
{
  cur = it.cur;
}


/** Increment iterator.
 * Advances to the next element. This is the infix-operator. It may be used
 * like this:
 * @code
 * for (MessageIterator cit = msgq->begin(); cit != msgq->end(); ++cit) {
 *   // your code here
 * }
 * @endcode
 * @return Reference to instance itself after advancing to the next element.
 */
MessageQueue::MessageIterator &
MessageQueue::MessageIterator::operator++()
{
  if ( cur != NULL )
    cur = cur->next;

  return *this;
}


/** Increment iterator.
 * Advances to the next element in allocated chunk list. This is the postfix-operator.
 * It may be used like this:
 * @code
 * for (MessageIterator cit = memmgr->begin(); cit != memmgr->end(); cit++) {
 *   // your code here
 * }
 * @endcode
 * Note that since a copy of the original iterator has to be created an returned it
 * the postfix operation takes both, more CPU time and more memory. If possible (especially
 * if used in a for loop like the example) use the prefix operator!
 * @see operator++()
 * @param inc ignored
 * @return copy of the current instance before advancing to the next element.
 */
MessageQueue::MessageIterator
MessageQueue::MessageIterator::operator++(int inc)
{
  MessageIterator rv(cur);
  if ( cur != NULL )
    cur = cur->next;
  
  return rv;
}


/** Advance by a certain amount.
 * Can be used to add an integer to the iterator to advance many steps in one go.
 * This operation takes linear time depending on i.
 * @param i steps to advance in list. If i is bigger than the number of remaining
 * elements in the list will stop beyond list.
 * @return reference to current instance after advancing i steps or after reaching
 * end of list.
 */
MessageQueue::MessageIterator &
MessageQueue::MessageIterator::operator+(unsigned int i)
{
  for (unsigned int j = 0; (cur != NULL) && (j < i); ++j) {
    cur = cur->next;
  }
  return *this;
}


/** Advance by a certain amount.
 * Works like operator+(unsigned int i), provided for convenience.
 * @param i steps to advance in list
 * @return reference to current instance after advancing i steps or after reaching
 * end of list.
 */
MessageQueue::MessageIterator &
MessageQueue::MessageIterator::operator+=(unsigned int i)
{
  for (unsigned int j = 0; (cur != NULL) && (j < i); ++j) {
    cur = cur->next;
  }
  return *this;
}


/** Check equality of two iterators.
 * Can be used to determine if two iterators point to the same chunk.
 * @param c iterator to compare current instance to
 * @return true, if iterators point to the same chunk, false otherwise
 */
bool
MessageQueue::MessageIterator::operator==(const MessageIterator & c) const
{
  return (cur == c.cur);
}


/** Check inequality of two iterators.
 * Can be used to determine if two iterators point to different chunks.
 * @param c iterator to compare current instance to
 * @return true, if iterators point to different chunks of memory, false otherwise
 */
bool
MessageQueue::MessageIterator::operator!=(const MessageIterator & c) const
{
  return (cur != c.cur);
}


/** Get memory pointer of chunk.
 * Use this operator to get the pointer to the chunk of memory that this iterator
 * points to.
 * @return pointer to memory
 */
Message *
MessageQueue::MessageIterator::operator*() const
{
  return ( cur != NULL ) ? cur->msg : NULL;
}


/** Act on current message.
 * Node that you have to make sure that this is not called on the end node!
 */
Message *
MessageQueue::MessageIterator::operator->() const
{
  return cur->msg;
}


/** Assign iterator.
 * Makes the current instance to point to the same memory element as c.
 * @param c assign value
 * @return reference to current instance
 */
MessageQueue::MessageIterator &
MessageQueue::MessageIterator::operator=(const MessageIterator & c)
{
  this->cur = c.cur;
  return *this;
}


/** Get ID of current element or 0 if element is end.
 * @return ID of current element or 0 if element is end.
 */
unsigned int
MessageQueue::MessageIterator::id() const
{
  if ( cur == NULL ) return 0;
  return cur->msg_id;
}
