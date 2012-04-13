/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/


#ifndef __WISELIB_UTIL_PSTL_LIST_DYNAMIC_H
#define __WISELIB_UTIL_PSTL_LIST_DYNAMIC_H

namespace wiselib {
	
	namespace list_dynamic_impl {
		template<
			typename Value_P,
			typename Allocator_P
		>
		struct DoublyConnectedListNode {
			typedef DoublyConnectedListNode<Value_P, Allocator_P> self_type;
			typename Allocator_P::template pointer_t<self_type> next;
			typename Allocator_P::template pointer_t<self_type> prev;
			Value_P value;
			Value_P& data() { return value; }
			const Value_P& data() const { return value; }
			
			DoublyConnectedListNode() { }
		};
	
	
		template<
			typename List_P
		>
		class list_dynamic_iterator {
			// {{{
			public:
				typedef List_P List;
				typedef typename List::Allocator Allocator;
				typedef typename List::value_type value_type;
				typedef typename List::node_type node_type;
				typedef value_type& reference;
				typedef value_type* pointer;
				typedef list_dynamic_iterator<List> iterator_type;
				typedef list_dynamic_iterator<List> self_type;
				typedef typename Allocator::template pointer_t<node_type> node_pointer_t;
				typedef typename Allocator::template pointer_t<self_type> self_pointer_t;
				
				list_dynamic_iterator() : list_(0) { }
				list_dynamic_iterator(List& l) : list_(&l) { }
				list_dynamic_iterator(List& l, const node_pointer_t node) : list_(&l), node_(node) { }
				list_dynamic_iterator(const self_type& other) : list_(other.list_), node_(other.node_) { }
				
				reference operator*() { return node_->value; }
				pointer operator->() { return &node_->value; }
				const reference operator*() const { return node_->value; }
				const pointer operator->() const { return &node_->value; }
				
				node_pointer_t node() { return node_; }
				List& list() { return *list_; }
				
				iterator_type& operator++() { node_ = node_->next; return *this; }
				
				iterator_type& operator--() {
					if(!node_) { node_ = list_->last(); }
					else { node_ = node_->prev; }
					return *this;
				}
				bool operator==(const iterator_type& other) const {
					return node_ == other.node_;
				}
				bool operator!=(const iterator_type& other) const {
					return node_ != other.node_;
				}
				
			private:
				List* list_;
				node_pointer_t node_;
		};
		// }}}
	} // ns
	
	
	
	
	/**
	 */
	template<
		typename OsModel_P,
		typename Value_P,
		typename Allocator_P
	>
	class list_dynamic {
		public:
			typedef list_dynamic_impl::DoublyConnectedListNode<Value_P, Allocator_P> Node_P;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::size_t size_t;
			typedef Value_P value_type;
			typedef value_type& reference;
			typedef const value_type& const_reference;
			typedef Allocator_P Allocator;
			typedef Node_P node_type;
			typedef typename Allocator::template pointer_t<node_type> node_pointer_t;
			typedef typename Allocator::template pointer_t<node_type> node_ptr_t;
			typedef list_dynamic<OsModel_P, Value_P, Allocator_P> self_type;
			typedef typename Allocator::template pointer_t<self_type> self_pointer_t;
			typedef list_dynamic_impl::list_dynamic_iterator<self_type> iterator;
			typedef list_dynamic_impl::list_dynamic_iterator<const self_type> const_iterator;
			
			list_dynamic() : allocator_(0), first_node_(0), last_node_(0), weak_(false) { };
			list_dynamic(Allocator& alloc) : allocator_(&alloc), first_node_(0), last_node_(0), weak_(false) { };
			list_dynamic(typename Allocator::self_pointer_t alloc) : allocator_(alloc), first_node_(0), last_node_(0), weak_(false) { };
			list_dynamic(const list_dynamic& other) : weak_(false) { *this = other; }
			
			~list_dynamic() {
				if(!weak_) {
					clear();
				}
			}
			
			list_dynamic& operator=(const self_type& other) {
				// TODO: Implement copy-on-write
				allocator_ = other.allocator_;
				clear();
				for(typename self_type::const_iterator iter = other.begin(); iter != other.end(); ++iter) {
					push_back(*iter);
				}
				return *this;
			}
			
			/**
			 * If true, don't delete the internal buffer upon destruction.
			 * Useful if you (shallow) "serialize" the string instance into some other
			 * format and call its destructor but actually plan to cast it
			 * back later. Normally that wouldnt be possible because the
			 * internally buffer would get lost, if you set the object to be
			 * "weak" in the meantime, the buffer will persist and the
			 * reconstructed object can be used.
			 * Only use if you know what you are doing! These methods are
			 * basically a recipe for memory leaks!
			 */
			bool weak() const { return weak_; }
			
			/**
			 * Set/unset "weak" property.
			 * See weak() for explanation on weakness.
			 */
			void set_weak(bool s) const { weak_ = s; }
			
			void set_allocator(Allocator& alloc) { allocator_ = &alloc; }
			
			iterator begin() { return iterator(*this, first_node_); }
			iterator end() { return iterator(*this); }
			
			const_iterator begin() const { return const_iterator(*this, first_node_); }
			const_iterator end() const { return const_iterator(*this); }
			
			bool empty() const { return begin() == end(); }
			
			size_t size() const {
				size_t s = 0;
				for(const_iterator i(begin()); i != end(); ++i) {
					s++;
				}
				return s;
			}
			
			const value_type& front() const { return first_node_->value; }
			const value_type& back() const { return last_node_->value; }
			value_type& front() { return first_node_->value; }
			value_type& back() { return last_node_->value; }
			iterator back_iterator() { return iterator(*this, last_node_); }
			
			iterator insert(iterator iter, const_reference v) {
				node_pointer_t n = allocator_-> template allocate<node_type>();
				n->value = v;
				
				iterator before(iter), after(iter);
				--before;
				if(before.node()) { before.node()->next = n; }
				else { first_node_ = n; }
				
				if(after.node()) { after.node()->prev = n; }
				else { last_node_ = n; }
				
				n->prev = before.node();
				n->next = after.node();
				
				iterator new_iter(*this, n);
				return new_iter;
			}
			
			node_pointer_t insert_n(const_reference v) {
				iterator iter = end();
				
				node_pointer_t n = allocator_-> template allocate<node_type>();
				n->value = v;
				
				iterator before(iter), after(iter);
				--before;
				if(before.node()) { before.node()->next = n; }
				else { first_node_ = n; }
				
				if(after.node()) { after.node()->prev = n; }
				else { last_node_ = n; }
				
				n->prev = before.node();
				n->next = after.node();
				
				return n;
			}
			
			iterator find(value_type v) {
				for(iterator i = begin(); i != end(); ++i) {
					if(*i == v) { return i; }
				}
				return end();
			}
			
			node_pointer_t find_n(value_type v) {
				return find(v).node();
			}
			
			iterator push_back(value_type v) {
				return insert(end(), v);
			}
			iterator push_front(value_type v) {
				return insert(begin(), v);
			}
			
			void pop_back() { erase(iterator(*this, last_node_)); }
			
			iterator erase(iterator iter) {
				if(!iter.node()) { return iter; }
				if(iter.node() == first_node_) { first_node_ = iter.node()->next; }
				if(iter.node() == last_node_) { last_node_ = iter.node()->prev; }
				if(iter.node()->next) { iter.node()->next->prev = iter.node()->prev; }
				if(iter.node()->prev) { iter.node()->prev->next = iter.node()->next; }
				
				iterator n(*this, iter.node()->next);
				allocator_-> template free<node_type>(iter.node());
				return n;
			}
			
			void swap(self_type& other) {
				node_pointer_t tmp_first = first_node_, tmp_last = last_node_;
				typename Allocator::self_pointer_t tmp_alloc = allocator_;
				
				first_node_ = other.first_node_;
				last_node_ = other.last_node_;
				allocator_ = other.allocator_;
				
				other.first_node_ = tmp_first;
				other.last_node_ = tmp_last;
				other.allocator_ = tmp_alloc;
			}
			
			
			void clear() {
				iterator rm = begin();
				while(rm.node()) { rm = erase(rm); }
			}
			
			node_pointer_t first() const { return first_node_; }
			node_pointer_t last() const { return last_node_; }
			
		private:
			typename Allocator::self_pointer_t allocator_;
			node_pointer_t first_node_, last_node_;
			mutable bool weak_;
	};
	
	
	template<
		typename OsModel_P,
		typename Allocator_P
	>
	struct maplist_adaptors {
		template<
			typename Value_P
		>
		class list_dynamic : public wiselib::list_dynamic<OsModel_P, Value_P, Allocator_P> {
		};
	};
	
} // ns

#endif // __WISELIB_UTIL_PSTL_LIST_DYNAMIC_H

