// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef ITERATIONPROXY_H
#define ITERATIONPROXY_H

#include <iterator>
#include <type_traits>

// The IterationProxy template provides access to the iterator interface of containers but hides the rest of it
// General version for containers without random access iterators
template <typename Container, typename T = void>
class IterationProxy {
public:
	IterationProxy(Container &container) :
		m_container(container) {}
	typename Container::iterator begin() { return m_container.begin(); }
	typename Container::iterator end() { return m_container.end(); }
	typename Container::const_iterator begin() const { return m_container.begin(); }
	typename Container::const_iterator end() const { return m_container.end(); }
	typename Container::const_iterator cbegin() { return m_container.cbegin(); }
	typename Container::const_iterator cend() { return m_container.cend(); }

private:
	Container &m_container;
};

// This specialized version is for containers that provide a random access iterator, we provide an operator[] in this case
template <typename Container>
class IterationProxy<Container,
	typename std::enable_if<std::is_same<typename std::iterator_traits<typename Container::iterator>::iterator_category,
		std::random_access_iterator_tag>::value>::type> {
public:
	IterationProxy(Container &container) :
		m_container(container) {}
	typename Container::iterator begin() { return m_container.begin(); }
	typename Container::iterator end() { return m_container.end(); }
	typename Container::const_iterator begin() const { return m_container.begin(); }
	typename Container::const_iterator end() const { return m_container.end(); }
	typename Container::const_iterator cbegin() { return m_container.cbegin(); }
	typename Container::const_iterator cend() { return m_container.cend(); }

	typename Container::reference operator[](int i) { return *(m_container.begin() + i); }
	typename Container::const_reference operator[](int i) const { return *(m_container.begin() + i); }

private:
	Container &m_container;
};

template <typename Container>
inline IterationProxy<Container> MakeIterationProxy(Container &container) { return IterationProxy<Container>(container); }

#endif
