/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __otbOGRFieldWrapper_txx
#define __otbOGRFieldWrapper_txx

/*===========================================================================*/
/*===============================[ Includes ]================================*/
/*===========================================================================*/
#include "otbOGRFieldWrapper.h"
#include <cassert>
#include <vector>
#include <boost/mpl/map.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/at.hpp>

#include <boost/static_assert.hpp>
#include <boost/range/size.hpp>
#include <boost/type_traits/is_same.hpp>
#include "boost/type_traits/is_contiguous.h" // from OTB actually

#include "ogr_feature.h" // OGRFeature::*field_getters


/*===========================================================================*/
/*================[ Associations C++ types -> OGR functions ]================*/
/*===========================================================================*/

namespace otb { namespace ogr {
/**\ingroup Geometry
 * \defgroup GeometryInternals Geometry Internals
 * \since OTB v 3.14.0
 */

/**\ingroup GeometryInternals
 * Namespace used to host internal meta-prog definitions.
 * \since OTB v 3.14.0
 */
namespace internal { // namespace internal
using namespace boost::mpl;

/**\ingroup GeometryInternals
 * Associative map of C++ types to OGR field types (\c OGRFieldType).
 * \internal Relies on Boost.MPL
 * \since OTB v 3.14.0
 * \todo \c OFTBinary, \c OFTDate, \c OFTTime and \c OFTDateTime are not managed
 * yet.
 */
typedef boost::mpl::map
  < pair<int, int_<OFTInteger> >
  , pair<std::vector<int>, int_<OFTIntegerList> >
  , pair<double, int_<OFTReal> >
  , pair<std::vector<double>, int_<OFTRealList> >
  , pair<std::string, int_<OFTString> >
  , pair<std::vector<std::string>, int_<OFTStringList> >
  // OFTBinary
  // OFTDate
  // OFTTime
  // OFTDateTime
  > FieldType_Map;

/**\ingroup GeometryInternals
 * \class MemberGetterPtr
 * Type for hosting simple member-function pointers to field getters.
 * \tparam T type of field according to OGR API.
 * \tparam ptr_to_function member function pointer to a field getter from \c
 *         OGRFeature.
 * \tparam FinalReturnType type of the field according to OTB wrappers (default
 * <tt> = T</tt>)
 *
 * \internal
 * This is a hack to pass a member function pointer as template-parameter.
 * \since OTB v 3.14.0
 */
template
  < typename T
  , T ( OGRFeature::*ptr_to_function )(int)
  , typename FinalReturnType = T
  > class MemberGetterPtr
    {
  public:
    static FinalReturnType call(OGRFeature &f, int index)
      {
      return (f.*ptr_to_function)(index);
      }
    };

/**\ingroup GeometryInternals
 * \class MemberSetterPtr
 * Type for hosting simple member-function pointers to field setters.
 * \tparam T type of field according to OGR API.
 * \tparam ptr_to_function member function pointer to a field setter from \c
 *         OGRFeature.
 * <tt> = T</tt>)
 *
 * \internal
 * This is a hack to pass a member function pointer as template-parameter.
 * \since OTB v 3.14.0
 */
template
  < typename T
  , void ( OGRFeature::*ptr_to_function )(int, T value)
  > class MemberSetterPtr
    {
  public:
    static void call(OGRFeature &f, int index, T const& value)
      {
      (f.*ptr_to_function)(index, value);
      }
    };

/**\ingroup GeometryInternals
 * \class MemberContainerGetterPtr
 * Type for hosting simple member-function pointers to list-field getters.
 * \tparam T type of field according to OGR API.
 * \tparam ptr_to_function member function pointer to a list-field getter from
 * \c OGRFeature.
 * \tparam FinalReturnType type of the list-field according to OTB wrappers
 * (default <tt> = std::vector<T></tt>)
 *
 * \internal
 * This is a hack to pass a member function pointer as template-parameter.
 * \since OTB v 3.14.0
 */
template
  < typename T
  , T const* ( OGRFeature::*ptr_to_function )(int, int*)
  , typename FinalReturnType = std::vector<T>
  > class MemberContainerGetterPtr
    {
  public:
    static FinalReturnType call(OGRFeature &f, int index)
      {
      int nb = 0;
      T const* raw_container = (f.*ptr_to_function)(index, &nb);
      const FinalReturnType res(raw_container+0, raw_container+nb);
      return res;
      }
    };

/*===========================================================================*/
/**\ingroup GeometryInternals
 * \class TagDispatchMemberContainerSetterPtr
 * \brief Dispatcher function for the Field Setter.
 * The container-field setters from OGR API have a C API. This dispatcher will
 * check whether the parameter container has a contiguous storage. If so it will
 * directly inject the address of the first element of the contiguous container
 * in the OGR C API. If not, the container will be converted into a container
 * with contiguous storage.
 * \since OTB v 3.14.0
 */
template
  < typename T
  , void ( OGRFeature::*ptr_to_function )(int, int, T*) // not const-correct
  , typename ActualParamType = std::vector<T>
  , bool Is_contiguous = boost::is_contiguous<ActualParamType>::value
  > class TagDispatchMemberContainerSetterPtr;

/**\ingroup GeometryInternals
 */
template
  < typename T
  , void ( OGRFeature::*ptr_to_function )(int, int, T*) // not const-correct
  , typename ActualParamType
  > class TagDispatchMemberContainerSetterPtr<T, ptr_to_function, ActualParamType, true>
    {
  public:
    static void call(OGRFeature &f, int index, ActualParamType const& container)
      {
      const int nb = boost::size(container);
      (f.*ptr_to_function)(index, nb, &container[0]);
      }
    };

/**\ingroup GeometryInternals
 */
template
  < typename T
  , void ( OGRFeature::*ptr_to_function )(int, int, T*) // not const-correct
  , typename ActualParamType
  > class TagDispatchMemberContainerSetterPtr<T, ptr_to_function, ActualParamType, false>
    {
  public:
    static void call(OGRFeature &f, int index, ActualParamType const& container)
      {
      const int nb = boost::size(container);
      std::vector<T> v(boost::begin(container), boost::end(container));
      (f.*ptr_to_function)(index, nb, &v[0]);
      }
    };

/**\ingroup GeometryInternals
 * \class MemberContainerSetterPtr
 * Type for hosting simple member-function pointers to list-field setters.
 * \tparam T type of field according to OGR API.
 * \tparam ptr_to_function member function pointer to a list-field setter from
 * \c OGRFeature.
 * \tparam FinalReturnType type of the list-field according to OTB wrappers
 * (default <tt> = std::vector<T></tt>)
 *
 * \internal
 * This is a hack to pass a member function pointer as template-parameter.
 * \since OTB v 3.14.0
 */
template
  < typename T
  , void ( OGRFeature::*ptr_to_function )(int, int, T*) // not const-correct
  , typename ActualParamType = std::vector<T>
  > class MemberContainerSetterPtr
    {
  public:
    static void call(OGRFeature &f, int index, ActualParamType const& container)
      {
      TagDispatchMemberContainerSetterPtr<
        T
        ,ptr_to_function
        ,ActualParamType
        , boost::is_contiguous<ActualParamType>::value
        >::call(f, index, container);
      }
    };

/**\ingroup GeometryInternals
 * Associative map of OGR field types (\c OGRFieldType) to their associated
 * getters.
 * \internal Relies on Boost.MPL
 * \since OTB v 3.14.0
 */
typedef map
  < pair<int_<OFTInteger>,     MemberGetterPtr<int,             &OGRFeature::GetFieldAsInteger> >
  , pair<int_<OFTIntegerList>, MemberContainerGetterPtr<int,    &OGRFeature::GetFieldAsIntegerList> >
  , pair<int_<OFTReal>,        MemberGetterPtr<double,          &OGRFeature::GetFieldAsDouble> >
  , pair<int_<OFTRealList>,    MemberContainerGetterPtr<double, &OGRFeature::GetFieldAsDoubleList> >
  , pair<int_<OFTString>,      MemberGetterPtr<char const*,     &OGRFeature::GetFieldAsString, std::string> >
  // , pair<int_<OFTStringList>,  MemberGetterPtr<char const*,     &OGRFeature::GetFieldAsString, std::string> >
  > FieldGetters_Map;

/**\ingroup GeometryInternals
 * Associative map of OGR field types (\c OGRFieldType) to their associated
 * setters.
 * \internal Relies on Boost.MPL
 * \since OTB v 3.14.0
 */
typedef map
  < pair<int_<OFTInteger>,     MemberSetterPtr<int,             &OGRFeature::SetField> >
  , pair<int_<OFTIntegerList>, MemberContainerSetterPtr<int,    &OGRFeature::SetField> >
  , pair<int_<OFTReal>,        MemberSetterPtr<double,          &OGRFeature::SetField> >
  , pair<int_<OFTRealList>,    MemberContainerSetterPtr<double, &OGRFeature::SetField> >
  // , pair<int_<OFTString>,      MemberSetterPtr<char const*,     &OGRFeature::SetField, std::string> >
  // , pair<int_<OFTStringList>,  MemberSetterPtr<char const*,     &OGRFeature::GetFieldAsString, std::string> >
  > FieldSetters_Map;

} // namespace internal
} } // end namespace otb::ogr

/*===========================================================================*/
/*=======================[ otb::ogr::Field functions ]=======================*/
/*===========================================================================*/

inline
void otb::ogr::Field::CheckInvariants() const
{
  assert(m_Feature && "OGR Fields must be associated to a valid feature");
  assert(int(m_index) < m_Feature->GetFieldCount() && "Out-of-range index for a OGR field");
  assert(m_Feature->GetFieldDefnRef(m_index) && "No definition available for the OGR field");
}

template <typename T>
inline
void otb::ogr::Field::SetValue(T const& value)
{
  CheckInvariants();
  const int VALUE = boost::mpl::at<internal::FieldType_Map, T>::type::value;
  typedef typename boost::mpl::at<internal::FieldType_Map, T>::type Kind;
  BOOST_STATIC_ASSERT(!(boost::is_same<Kind, boost::mpl::void_>::value));
  assert(m_Definition.GetType() == Kind::value && "OGR field type mismatches the type of new field value");
  typedef typename boost::mpl::at<internal::FieldSetters_Map, Kind>::type SetterType;
  BOOST_STATIC_ASSERT(!(boost::is_same<SetterType, boost::mpl::void_>::value));
  SetterType::call(*m_Feature, m_index, value);
}

template <typename T>
inline
T otb::ogr::Field::GetValue() const
{
  CheckInvariants();
  assert(HasBeenSet() && "Cannot access the value of a field that hasn't been set");
  const int VALUE = boost::mpl::at<internal::FieldType_Map, T>::type::value;
  typedef typename boost::mpl::at<internal::FieldType_Map, T>::type Kind;
  BOOST_STATIC_ASSERT(!(boost::is_same<Kind, boost::mpl::void_>::value));
  assert(m_Definition.GetType() == Kind::value && "OGR field type mismatches the type of requested field value");
  typedef typename boost::mpl::at<internal::FieldGetters_Map, Kind>::type GetterType;
  BOOST_STATIC_ASSERT(!(boost::is_same<GetterType, boost::mpl::void_>::value));
  return GetterType::call(*m_Feature, m_index);
}

#endif // __otbOGRFieldWrapper_txx