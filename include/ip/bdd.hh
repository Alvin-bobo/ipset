/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2010, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the LICENSE.txt file in this distribution for license
 * details.
 * ----------------------------------------------------------------------
 */

#ifndef IP__BDD_HH
#define IP__BDD_HH


/**
 * @file
 *
 * This file contains a very basic BDD implementation for storing IP
 * sets.
 */

#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/variant.hpp>


namespace ip {
namespace bdd {


/**
 * Each variable in a BDD is referred to by number.
 */

typedef unsigned int  variable_t;


// Forward declare our classes.

template <typename Value> class terminal_t;
template <typename Value> class nonterminal_t;
template <typename Value> class node_t;


/**
 * Provides more convenient access to the BDD node types for a
 * particular terminal value type.  We use shared pointers to refer to
 * nodes, since they will (in most cases) be “reduced” — we will
 * ensure that two nodes with the same value will always live at the
 * same memory location.
 */

template <typename Value>
struct types
{
    /**
     * A shared pointer to a terminal node.
     */

    typedef boost::shared_ptr<terminal_t<Value> >  terminal;

    /**
     * A shared pointer to a nonterminal node.
     */

    typedef boost::shared_ptr<nonterminal_t<Value> >  nonterminal;

    /**
     * A shared pointer to a node which might be terminal or
     * nonterminal.
     */

    typedef node_t<Value>  node;
};


/**
 * A terminal node.  This is a leaf of the BDD tree.  BDDs map
 * variable assignments to values.  Each variable in the assignment is
 * boolean; the value is user-specified.
 *
 * This class does not take care of ensuring that all BDD nodes are
 * reduced; that is handled by the node_cache_t class.
 */

template <typename Value>
class terminal_t
{
private:
    /**
     * The value contained by this terminal node.
     */

    Value  _value;

public:
    /**
     * Create a new terminal node containing the given value.  Most
     * user code should not create terminal_t objects by hand;
     * instead, they should use the node_cache_t class.
     *
     * Note that we do not ensure that terminal nodes are reduced:
     * it's perfectly valid to instantiate two terminal_t objects by
     * hand with the same value.  If you do so, those two objects will
     * live at different memory locations, and won't be reduced.
     */

    explicit terminal_t(Value value_):
        _value(value_)
    {
    }

    /**
     * Return the value contained by this terminal node.
     */

    Value value() const
    {
        return _value;
    }

    /**
     * Test two terminals for equality.  Note that this operator is
     * usually not the one that will be used; reduced nodes are
     * compared by their memory address, which ends up using
     * shared_ptr's implementation of the operator.
     */

    bool operator == (const terminal_t &other) const
    {
        return _value == other._value;
    }

    /**
     * Compare two terminals by value.  Note that this operator is
     * usually not the one that will be used; reduced nodes are
     * compared by their memory address, which ends up using
     * shared_ptr's implementation of the operator.
     */

    bool operator < (const terminal_t &other) const
    {
        return _value < other._value;
    }
};


/**
 * A nonterminal node.  This is an inner node of the BDD tree.  The
 * node represents one variable in an overall variable assignment.
 * The node has two children: a “low” child and a “high” child.  The
 * low child is the subtree that applies when the node's variable is
 * false or 0; the high child is the subtree that applies when it's
 * true or 1.
 *
 * This class does not take care of ensuring that all BDD nodes are
 * reduced; that is handled by the node_cache_t class.
 */

template <typename Value>
class nonterminal_t
{
private:
    // Shorthand typedefs

    typedef typename types<Value>::node  node_t;

    /**
     * The variable that this node represents.
     */

    variable_t  _variable;

    /**
     * The subtree node for when the variable is false.
     */

    node_t  _low;

    /**
     * The subtree node for when the variable is true.
     */

    node_t  _high;

public:
    /**
     * Create a new nonterminal node for the given variable and
     * subtrees.  Most user code should not create nonterminal_t
     * objects by hand; instead, they should use the node_cache_t
     * class.
     *
     * Note that we do not ensure that nonterminal nodes are reduced:
     * it's perfectly valid to instantiate two nonterminal_t objects
     * by hand with the same contents.  If you do so, those two
     * objects will live at different memory locations, and won't be
     * reduced.
     */

    nonterminal_t(variable_t variable_,
                  node_t low_,
                  node_t high_):
        _variable(variable_),
        _low(low_),
        _high(high_)
    {
    }

    /**
     * Return the variable that this node represents.
     */

    variable_t variable() const
    {
        return _variable;
    }

    /**
     * Return the subtree node for when the variable is false.
     */

    node_t low() const
    {
        return _low;
    }

    /**
     * Return the subtree node for when the variable is true.
     */

    node_t high() const
    {
        return _high;
    }

    /**
     * Test two nonterminals for equality.  Note that this operator is
     * usually not the one that will be used; reduced nodes are
     * compared by their memory address, which ends up using
     * shared_ptr's implementation of the operator.
     */

    bool operator == (const nonterminal_t &other) const
    {
        return
            (_variable == other._variable) &&
            (_low == other._low) &&
            (_high == other._high);
    }

    /**
     * Compare two nonterminals by value.  Note that this operator is
     * usually not the one that will be used; reduced nodes are
     * compared by their memory address, which ends up using
     * shared_ptr's implementation of the operator.
     */

    bool operator < (const nonterminal_t &other) const
    {
        if (_variable < other._variable)
            return true;
        else if (_variable > other._variable)
            return false;

        if (_low < other._low)
            return true;
        else if (_low > other._low)
            return false;

        return (_high < other._high);
    }

    /**
     * Apply a Boost Variant visitor to the low subtree of this node.
     */

    template <typename Visitor>
    typename Visitor::result_type apply_low(Visitor &visitor)
    {
        return boost::apply_visitor(visitor, _low);
    }

    /**
     * Apply a Boost Variant visitor to the high subtree of this node.
     */

    template <typename Visitor>
    typename Visitor::result_type apply_high(Visitor &visitor)
    {
        return boost::apply_visitor(visitor, _high);
    }
};


/**
 * A node that might be terminal or nonterminal.  This is basically
 * just a wrapper around a Boost Variant, with some helper methods
 * that reduce typing when accessing the details of the node.
 */

template <typename Value>
class node_t
{
private:
    // Shorthand typedefs

    typedef typename types<Value>::terminal  terminal_p;
    typedef typename types<Value>::nonterminal  nonterminal_p;

public:
    /**
     * The Boost Variant type used to store the terminal or
     * nonterminal for this node.  Note that these are shared
     * pointers, so that we can correctly use reduced nodes when
     * node_t instances are copied around.
     */

    typedef boost::variant<terminal_p, nonterminal_p>  variant_t;

private:
    /**
     * The terminal or nonterminal for this node.
     */

    variant_t  _variant;

public:
    /**
     * Create a new node_t for a terminal node.
     */

    node_t(terminal_p terminal):
        _variant(terminal)
    {
    }

    /**
     * Create a new node_t for a nonterminal node.
     */

    node_t(nonterminal_p nonterminal):
        _variant(nonterminal)
    {
    }

    /**
     * Create a copy of a node.  Since we use shared pointers, the
     * underlying terminal_t or nonterminal_t is not copied; instead,
     * we get a new shared_ptr to it, with an increased reference
     * count.
     */

    node_t(const node_t &other):
        _variant(other._variant)
    {
    }

    /**
     * Assign a copy of a node.  Since we use shared pointers, the
     * underlying terminal_t or nonterminal_t is not copied; instead,
     * we get a new shared_ptr to it, with an increased reference
     * count.
     */

    node_t &
    operator = (const node_t &other)
    {
        _variant = other._variant;
        return (*this);
    }

    /**
     * Test two nodes for equality.  Since we use shared pointers,
     * nodes are compared by their memory address, which gives us the
     * correct semantics for reduced nodes.
     */

    bool operator == (const node_t &other) const
    {
        return _variant == other._variant;
    }

    /**
     * Compare two nodes.  Since we use shared pointers, nodes are
     * compared by their memory address, which gives us the correct
     * semantics for reduced nodes.
     */

    bool operator < (const node_t &other) const
    {
        return _variant < other._variant;
    }

    /**
     * Coerce this node into a terminal.  If the node isn't a
     * terminal, Boost's bad_get exception will be thrown.
     */

    terminal_p as_terminal() const
    {
        return boost::get<terminal_p>(_variant);
    }

    /**
     * Coerce this node into a nonterminal.  If the node isn't a
     * nonterminal, Boost's bad_get exception will be thrown.
     */

    nonterminal_p as_nonterminal() const
    {
        return boost::get<nonterminal_p>(_variant);
    }

    /**
     * Return the Boost Variant instance for this node.
     */

    variant_t variant() const
    {
        return _variant;
    }
};


template <typename Value>
std::ostream &
operator << (std::ostream &stream, const node_t<Value> &node)
{
    stream << node.variant();
    return stream;
}


/**
 * A node cache.  This class is responsible for ensuring that BDD
 * nodes are reduced — that is, that we only ever instantiate one
 * terminal or nonterminal with a particular value.
 */

template <typename Value>
class node_cache_t
{
private:
    // Shorthand typedefs

    typedef typename types<Value>::terminal  terminal_p;
    typedef typename types<Value>::nonterminal  nonterminal_p;
    typedef typename types<Value>::node  node_t;

    /**
     * The type of the terminal cache.  Terminals are instantiated
     * with the contained value, which is the key of the cache.  The
     * value of the cache is a shared pointer to a terminal_t object.
     */

    typedef std::map<Value, terminal_p>
    terminal_map_t;

    /**
     * The type of the nonterminal map's key.  Nonterminals are
     * constructed from three values: the variable, the low subtree,
     * and the high subtree.  The key of the nonterminal cache is a
     * tuple of these three values.
     */

    typedef boost::tuple<variable_t, node_t, node_t>
    nonterminal_key_t;

    /**
     * The type of the nonterminal cache.  The key of the cache is the
     * nonterminal_key_t type.  The value of the cache is a shared
     * pointer to a nonterminal_t object.
     */

    typedef std::map<nonterminal_key_t, nonterminal_p>
    nonterminal_map_t;

    /**
     * The cache of terminal nodes.
     */

    terminal_map_t  terminal_map;

    /**
     * The cache of nonterminal nodes.
     */

    nonterminal_map_t  nonterminal_map;

public:
    /**
     * Create a new, empty node cache.
     */

    node_cache_t()
    {
    }

    /**
     * Create or retrieve the terminal for the specified value.  If
     * the cache already contains the corresponding terminal, the
     * existing instance will be returned; otherwise, we create and
     * return a new instance.
     */

    node_t terminal(Value value)
    {
        typename terminal_map_t::iterator  lb =
            terminal_map.lower_bound(value);

        if ((lb != terminal_map.end()) && (value == lb->first))
        {
            return lb->second;
        } else {
            terminal_p  terminal(new terminal_t<Value>(value));

            terminal_map.insert
                (lb, typename terminal_map_t::value_type
                 (value, terminal));

            return terminal;
        }
    }

    /**
     * Create or retrieve the nonterminal for the specified values.
     * If the cache already contains the corresponding terminal, the
     * existing instance will be returned; otherwise, we create and
     * return a new instance.
     */

    node_t nonterminal(variable_t variable,
                       node_t low,
                       node_t high)
    {
        if (low == high)
            return low;

        nonterminal_key_t  key =
            boost::make_tuple(variable, low, high);

        typename nonterminal_map_t::iterator  lb =
            nonterminal_map.lower_bound(key);

        if ((lb != nonterminal_map.end()) && (key == lb->first))
        {
            return lb->second;
        } else {
            nonterminal_p  nonterminal
                (new nonterminal_t<Value>(variable, low, high));

            nonterminal_map.insert
                (lb, typename nonterminal_map_t::value_type
                 (key, nonterminal));

            return nonterminal;
        }
    }
};


} // namespace bdd
} // namespace ip

#endif /* IP__BDD_HH */
