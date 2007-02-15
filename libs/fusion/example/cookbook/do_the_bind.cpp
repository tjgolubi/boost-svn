/*=============================================================================
    Copyright (c) 2006-2007 Tobias Schwinger
  
    Use modification and distribution are subject to the Boost Software 
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt).

    Problem:

    How to "do the Bind?"

    This recipe shows how to implement a function binder, similar to 
    Boost.Bind based on the Functional module of Fusion.

    It works as follows:

    'bind' is a global, stateless function object. It is implemented in
    fused form (fused_binder) and transformed into a variadic function 
    object. When called, 'bind' returns another function object, which
    holds the arguments of the call to 'bind'. It is, again, implemented 
    in fused form (fused_bound_function) and transformed into unfused
    form. 
==============================================================================*/

#include <boost/fusion/functional/invoke.hpp>
#include <boost/fusion/functional/unfused_generic.hpp>
#include <boost/fusion/functional/unfused_rvalue_args.hpp>
#include <boost/fusion/functional/storable_arguments.hpp>

#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/sequence/intrinsic/mpl.hpp>
#include <boost/fusion/sequence/intrinsic/front.hpp>
#include <boost/fusion/sequence/intrinsic/empty.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/algorithm/transformation/pop_front.hpp>

#include <boost/type_traits/remove_reference.hpp>

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/count_if.hpp>

#include <boost/ref.hpp>
#include <iostream>
#include <typeinfo>

namespace impl
{
    namespace fusion = boost::fusion;
    namespace result_of = boost::fusion::result_of;
    namespace mpl = boost::mpl;
    using mpl::placeholders::_;

    // Placeholders (we inherit from mpl::int_, so we can use placeholders
    // as indices for fusion::at, later) 
    template <int I> struct placeholder : mpl::int_<I> { };

    // A traits class to find out whether T is a placeholeder
    template <typename T> struct is_placeholder              : mpl::false_  { };
    template <int I> struct is_placeholder< placeholder<I> > : mpl::true_   { };
    template <int I> struct is_placeholder< placeholder<I> & > : mpl::true_   { };
    template <int I> struct is_placeholder< placeholder<I> const   > : mpl::true_   { };
    template <int I> struct is_placeholder< placeholder<I> const & > : mpl::true_   { };

    // This class template provides a Polymorphic Function Object to be used
    // with fusion::transform. It is applied to the sequence of arguments that
    // describes the binding and holds a reference to the sequence of arguments 
    // from the final call. 
    template<class FinalArgs> struct argument_transform
    {
        FinalArgs const & ref_final_args;
    public:

        explicit argument_transform(FinalArgs const & final_args)
            : ref_final_args(final_args)
        { }

        // A placeholder? Replace it with an argument from the final call...
        template <int Index>
        inline typename result_of::at_c<FinalArgs const, Index>::type
        operator()(placeholder<Index> const &) const
        {
            return fusion::at_c<Index>(this->ref_final_args);
        }
        // ...just return the bound argument, otherwise.
        template <typename T> inline T & operator()(T & bound) const
        {
            return bound;
        }

        template <typename T>
        struct result
            : mpl::eval_if< is_placeholder<T>, 
                result_of::at<FinalArgs,typename boost::remove_reference<T>::type>,
                mpl::identity<T>
            >
        { };
    };

    // Fused implementation of the bound function, the function object 
    // returned by bind
    template <class BindArgs> class fused_bound_function 
    {
        BindArgs fsq_bind_args;
    public:

        fused_bound_function(BindArgs const & bind_args)
          : fsq_bind_args(bind_args)
        { }

        template <class FinalArgs, bool Enable =
        // Disable for empty sequences if we expect arguments 
            ! (result_of::empty<FinalArgs>::value &&
                !! mpl::count_if<BindArgs,is_placeholder<_> >::value) > 
        struct result
        { };

        template <class FinalArgs>
        struct result<FinalArgs,true>
            : result_of::invoke< typename result_of::front<BindArgs>::type,
                typename result_of::transform<
                    typename result_of::pop_front<BindArgs>::type,
                    argument_transform<FinalArgs> const 
                >::type
            >
        { }; 

        template <class FinalArgs>
        inline typename result<FinalArgs>::type 
        operator()(FinalArgs const & final_args) const
        {
            return fusion::invoke( fusion::front(this->fsq_bind_args),
                fusion::transform( fusion::pop_front(this->fsq_bind_args),
                    argument_transform<FinalArgs>(final_args) ) );
        }
        // Could add a non-const variant - omitted for readability

    };

    // Fused implementation of the 'bind' function
    struct fused_binder
    {
        template <class BindArgs>
        struct result
        {
            // We have to transform the arguments so they are held by-value
            // in the returned function. 
            typedef fusion::unfused_generic< fused_bound_function<
                typename fusion::storable_arguments<BindArgs>::type > > type;
        };

        template <class BindArgs>
        inline typename result<BindArgs>::type 
        operator()(BindArgs & bind_args) const
        {
            return typename result<BindArgs>::type(bind_args);
        }
    };

    // The binder's unfused type. We use unfused_rvalue_args to make that
    // thing more similar to Boost.Bind. Because of that we have to use 
    // Boost.Ref (below in the sample code)
    typedef fusion::unfused_rvalue_args<fused_binder> binder;
}

// Placeholder globals
impl::placeholder<0> const _1_ = impl::placeholder<0>();
impl::placeholder<1> const _2_ = impl::placeholder<1>();
impl::placeholder<2> const _3_ = impl::placeholder<2>();
impl::placeholder<3> const _4_ = impl::placeholder<3>();

// The bind function is a global, too
impl::binder const bind = impl::binder();


// OK, let's try it out:

struct func
{
    typedef int result_type;

    inline int operator()() const
    {
        std::cout << "operator()" << std::endl;
        return 0;
    }

    template <typename A> 
    inline int operator()(A const & a) const
    {
        std::cout << "operator()(A const & a)" << std::endl;
        std::cout << "  a = " << a << "  A = " << typeid(A).name() << std::endl;
        return 1;
    }

    template <typename A, typename B> 
    inline int operator()(A const & a, B & b) const
    {
        std::cout << "operator()(A const & a, B & b)" << std::endl;
        std::cout << "  a = " << a << "  A = " << typeid(A).name() << std::endl;
        std::cout << "  b = " << b << "  B = " << typeid(B).name() << std::endl;
        return 2;
    }
};

int main()
{
    func f;
    int value = 42;
    using boost::ref;

    int errors = 0;
    errors += !( bind(f)() == 0);
    errors += !( bind(f,"Hi")() == 1);
    errors += !( bind(f,_1_)("there.") == 1);
    errors += !( bind(f,"The answer is",_1_)(value) == 2);
    errors += !( bind(f,_1_,ref(value))("Really?") == 2);
    errors += !( bind(f,_1_,_2_)("Dunno. If there is an answer, it's",value) == 2);

    return !! errors;
}

