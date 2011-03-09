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

#ifndef __ReciprocalHAlphaImageFilter_h
#define __ReciprocalHAlphaImageFilter_h

#include "otbUnaryFunctorImageFilter.h"
#include "otbHermitianEigenAnalysis.h"
#include "itkVector.h"
#include "otbMath.h"

namespace otb
 {

namespace Functor {

/** \class otbHAlphaFunctor
 * \brief Evaluate the H-Alpha parameters from the reciprocal coherency matrix image
 *
 *   Output value are:
 *   channel #0 : entropy
 *   channel #1 : \f$ \alpha \f$ parameter
 *   channel #2 : anisotropy
 *
 * \ingroup SARPolarimetry
 *
 */
template< class TInput, class TOutput>
class ReciprocalHAlphaFunctor
{
public:
  typedef typename std::complex <double>         ComplexType;
  typedef typename TOutput::ValueType              OutputValueType;

  /** CoherencyMatrix type **/
  typedef itk::Vector<double, 9> CoherencyMatrixType;

  /** Vector type used to store eigenvalues. */
  typedef itk::Vector<double, 3> EigenvalueType;

  /** Matrix type used to store eigenvectors. */
  typedef itk::Vector<float, 2> VectorType;
  typedef itk::Vector<VectorType, 3> EigenVectorFirstComposantType;
  typedef itk::Vector<VectorType, 3> EigenVectorType; // eigenvector type (real part, imaginary part)
  typedef itk::Vector<itk::Vector<float, 6>, 3> EigenMatrixType;
  typedef itk::Image<EigenVectorType, 2> EigenVectorImageType;
  typedef itk::Image<double, 2> EigenValueImageType;

  typedef itk::Vector<double, 3> OutputVectorType;

  typedef itk::Vector<float, 2> ComplexVectorType;
  typedef itk::Vector<ComplexVectorType, 3> HermitianVectorType;
  typedef itk::Vector<HermitianVectorType, 3> HermitianMatrixType;
  typedef otb::HermitianEigenAnalysis<CoherencyMatrixType, EigenvalueType, EigenMatrixType> HermitianAnalysisType;


  inline TOutput operator()( const TInput & Coherency ) const
    {
    TOutput result;
    result.SetSize(m_NumberOfComponentsPerPixel);
 
    CoherencyMatrixType T;
    EigenvalueType eigenValues;
    EigenMatrixType eigenVectors;
    HermitianAnalysisType HermitianAnalysis;

    T[0] = static_cast<double>(Coherency[0].real());
    T[1] = static_cast<double>(Coherency[3].real());
    T[2] = static_cast<double>(Coherency[5].real());
    T[3] = static_cast<double>(Coherency[1].real());
    T[4] = static_cast<double>(Coherency[1].imag());
    T[5] = static_cast<double>(Coherency[2].real());
    T[6] = static_cast<double>(Coherency[2].imag());
    T[7] = static_cast<double>(Coherency[4].real());
    T[8] = static_cast<double>(Coherency[4].imag());
    HermitianAnalysis.ComputeEigenValuesAndVectors(T, eigenValues, eigenVectors);

    // Entropy estimation
    double  totalEigenValues(0.0);
    double  p[3];
    double entropy;
    double alpha;
    double anisotropy;

    totalEigenValues = static_cast<double>( eigenValues[0] + eigenValues[1] + eigenValues[2]);
    if (totalEigenValues <m_Epsilon)
      {
        totalEigenValues = m_Epsilon;
      }

    for (unsigned int k = 0; k < 3; k++)
      {
        if (eigenValues[k] <0.)
          {
            eigenValues[k] = 0.;
          }

        p[k] = static_cast<double>(eigenValues[k]) / totalEigenValues;
      }

    if ( (p[0] < m_Epsilon) || (p[1] < m_Epsilon) || (p[2] < m_Epsilon) )
      {
        entropy =0.0;
      }
    else
      {
        entropy = p[0]*log(p[0]) + p[1]*log(p[1]) + p[2]*log(p[2]);
        entropy /= -log(3.);
      }

    // alpha estimation
    double val0, val1, val2;
    double a0, a1, a2;

    for(unsigned int k = 0; k < 3; k++)
      {
         p[k] = eigenValues[k] / totalEigenValues;

         if (p[k] < 0.)
           {
             p[k] = 0.;
           }

         if (p[k] > 1.)
           {
             p[k] = 1.;
           }
      }

    val0=sqrt(static_cast<double>(eigenVectors[0][0]*eigenVectors[0][0]) + static_cast<double>(eigenVectors[0][1]*eigenVectors[0][1]));
    a0=acos(vcl_abs(val0)) * CONST_180_PI;

    val1=sqrt(static_cast<double>(eigenVectors[0][2]*eigenVectors[0][2]) + static_cast<double>(eigenVectors[0][3]*eigenVectors[0][3]));
    a1=acos(vcl_abs(val1)) * CONST_180_PI;

    val2=sqrt(static_cast<double>(eigenVectors[0][4]*eigenVectors[0][4]) + static_cast<double>(eigenVectors[0][5]*eigenVectors[0][5]));
    a2=acos(vcl_abs(val2)) * CONST_180_PI;

    alpha=p[0]*a0 + p[1]*a1 + p[2]*a2;

    if (alpha>90)
      {
        alpha=0.;
      }

    // Anisotropy estimation
    anisotropy=(eigenValues[1] - eigenValues[2])/(eigenValues[1] + eigenValues[2]+m_Epsilon);

    result[0] = entropy;
    result[1] = alpha;
    result[2] = anisotropy;

    return result;
    }

   unsigned int GetOutputSize()
   {
     return m_NumberOfComponentsPerPixel;
   }

   /** Constructor */
   ReciprocalHAlphaFunctor() : m_Epsilon(1e-6) {}

   /** Destructor */
   virtual ~ReciprocalHAlphaFunctor() {}

private:
   itkStaticConstMacro(m_NumberOfComponentsPerPixel, unsigned int, 3);
   const double m_Epsilon;
};
}


/** \class otbHAlphaImageFilter
 * \brief Compute the H-Alpha image (3 channels)
 * from the Reciprocal coherency image (6 complex channels)
 */
template <class TInputImage, class TOutputImage, class TFunction = Functor::ReciprocalHAlphaFunctor<
    ITK_TYPENAME TInputImage::PixelType, ITK_TYPENAME TOutputImage::PixelType> >
class ITK_EXPORT ReciprocalHAlphaImageFilter :
   public otb::UnaryFunctorImageFilter<TInputImage, TOutputImage, TFunction>
{
public:
   /** Standard class typedefs. */
   typedef ReciprocalHAlphaImageFilter  Self;
   typedef otb::UnaryFunctorImageFilter<TInputImage, TOutputImage, TFunction> Superclass;
   typedef itk::SmartPointer<Self>        Pointer;
   typedef itk::SmartPointer<const Self>  ConstPointer;

   /** Method for creation through the object factory. */
   itkNewMacro(Self);

   /** Runtime information support. */
   itkTypeMacro(ReciprocalHAlphaImageFilter, UnaryFunctorImageFilter);

protected:
   ReciprocalHAlphaImageFilter() {}
  virtual ~ReciprocalHAlphaImageFilter() {}

private:
  ReciprocalHAlphaImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&);            //purposely not implemented

};
  
} // end namespace otb

#endif
