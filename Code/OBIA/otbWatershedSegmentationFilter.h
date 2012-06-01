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
#ifndef __otbWatershedSegmentationFilter_h
#define __otbWatershedSegmentationFilter_h

#include "itkMacro.h"
#include "itkCastImageFilter.h"
#include "itkWatershedImageFilter.h"

namespace otb {

/** \class WatershedSegmentationFilter
*
*  
*/


template <class TInputImage,  class TOutputLabelImage >
class WatershedSegmentationFilter 
  : public itk::ImageToImageFilter<TInputImage, TOutputLabelImage>
{
public:
  /** Standard Self typedef */
  typedef WatershedSegmentationFilter                             Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputLabelImage> Superclass;
  typedef itk::SmartPointer<Self>                                 Pointer;
  typedef itk::SmartPointer<const Self>                           ConstPointer;

  /** Some convenient typedefs. */
  typedef TInputImage                                             InputImageType;
  typedef TOutputLabelImage                                       OutputLabelImageType;
  
  typedef itk::WatershedImageFilter<TInputImage>                  WatershedFilterType;
  typedef typename WatershedFilterType::OutputImageType           InternalOutputImageType;

  typedef itk::CastImageFilter<InternalOutputImageType,
                               OutputLabelImageType>              CastImageFilterType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(WatershedSegmentationFilter, ImageToImageFilter);

  /** ImageDimension constants */
  itkStaticConstMacro(ImageDimension, unsigned int, TInputImage::ImageDimension);

  otbSetObjectMemberMacro(WatershedFilter,Level,float);
  otbGetObjectMemberConstReferenceMacro(WatershedFilter,Level,float);

  otbSetObjectMemberMacro(WatershedFilter,Threshold,float);
  otbGetObjectMemberConstReferenceMacro(WatershedFilter,Threshold,float);


protected:
  WatershedSegmentationFilter();

  virtual ~WatershedSegmentationFilter();

  virtual void GenerateData();

private:
  typename CastImageFilterType::Pointer m_CastFilter;
  typename WatershedFilterType::Pointer m_WatershedFilter;
};


} // end namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbWatershedSegmentationFilter.txx"
#endif

#endif