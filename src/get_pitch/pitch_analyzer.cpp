/// @file

#include <iostream>
#include <math.h>
#include "pitch_analyzer.h"

using namespace std;

/// Name space of UPC
namespace upc {
  void PitchAnalyzer::autocorrelation(const vector<float> &x, vector<float> &r) const {

    for (unsigned int l = 0; l < r.size(); ++l) {
  		/// \TODO Compute the autocorrelation r[l]
      /// Para cada TODO en el codigo, añadir comando
      /// \FET Hemos hecho la autocorrelacion sesgada
      /// \f[
      /// r_{xx}[m]=\frac{1}{N} \sum_{n=0}^{N-m} x[n] x[n+m]
      /// \f]
      r[l]=0;
      for(unsigned int n=l; n< x.size(); n++){
        r[l] += x[n]*x[n-l];
    
    }
    r[l] /= x.size();
    }
    
    
    if (r[0] == 0.0F) //to avoid log() and divide zero 
      r[0] = 1e-10; 
  }

  void PitchAnalyzer::set_window(Window win_type) {
    if (frameLen == 0)
      return;

    window.resize(frameLen);

    switch (win_type) {
    case HAMMING:
      /// \TODO Implement the Hamming window

      /**
       * \DONE Se ha implementado la ventana de Hamming.
       * - Se calcula el coeficiente `a0` y `a1` necesarios para la ventana de Hamming.
       * - Luego, se itera sobre cada índice `i` de la ventana y se calcula el valor correspondiente usando la fórmula de la ventana de Hamming: 
       * - \f$ w[i] = a_0 - a_1 \cdot \cos\left(\frac{2\pi i}{N-1}\right) \f$ donde \f$ N \f$ es el tamaño de la ventana y \f$ a_0 \f$ y \f$ a_1 \f$ son los coeficientes de la ventana de Hamming.
       */

      for(unsigned int i=0; i<frameLen; i++){
        float a0 = HAMMING_A0;
        float a1 = 1 - a0;
        window[i] = a0 - a1*cos((2*M_PI*i)/(frameLen-1));
      }
      break;
    case RECT:
    default:
      window.assign(frameLen, 1);
    }
  }

  void PitchAnalyzer::set_f0_range(float min_F0, float max_F0) {
      npitch_min = (unsigned int) (samplingFreq / max_F0);
      if (npitch_min < 2)
          npitch_min = 2; // samplingFreq/2

      npitch_max = 1 + (unsigned int) (samplingFreq / min_F0);

      // frameLen should include at least 2*T0
      if (npitch_max > frameLen / 2)
          npitch_max = frameLen / 2;
    }

bool PitchAnalyzer::unvoiced(float zcr, float r1norm, float rmaxnorm, float pot) const {
    /// \TODO Implement a rule to decide whether the sound is voiced or not.
    /// * You can use the standard features (pot, r1norm, rmaxnorm),
    ///   or compute and use other ones.

    // Rule to determine if the sound is voiced or unvoiced
    // Count the number of conditions satisfied
    int voiced = 0;                 

    // Check if the maximum autocorrelation value is above a threshold
    if(rmaxnorm > r2maxth) voiced++; 

    // Check if the first normalized autocorrelation value is above a threshold
    if(r1norm > r1r0th) voiced++;    

    // Check if the zero crossing rate (ZCR) is below a threshold
    if(zcr < zcrth) voiced++;         

    // Check if the power is above a threshold
    if(pot > potth) voiced++;

    // If all conditions are satisfied, consider the sound as voiced
    if(voiced == 4) return false;  // Voiced
    return true;                    // Unvoiced

    /**
     * \DONE Se ha implementado una regla para decidir si el sonido es sonoro o no.
     * - Se han proporcionado comentarios para explicar cómo se determina si un sonido es sonoro o no.
     * - Se ha añadido una explicación clara de las condiciones utilizadas y cómo afectan la clasificación del sonido.
     */
}




  float PitchAnalyzer::compute_pitch(vector<float>& x, float zcr) const {
    if (x.size() != frameLen)
      return -1.0F;

    //Window input frame
    for (unsigned int i=0; i<x.size(); ++i)
      x[i] *= window[i];

    vector<float> r(npitch_max);

    //Compute correlation
    autocorrelation(x, r);

    vector<float>::const_iterator iR = r.begin(), iRMax = iR;

    /// \TODO 
	  /// Find the lag of the maximum value of the autocorrelation away from the origin.<br>
	  /// Choices to set the minimum value of the lag are:
	  ///    - The first negative value of the autocorrelation.
	  ///    - The lag corresponding to the maximum value of the pitch.
    ///	   .
	  /// In either case, the lag should not exceed that of the minimum value of the pitch.


    for(iRMax = iR = r.begin() + npitch_min; iR < r.begin() + npitch_max; iR++){ 
      // The maximum value of the autocorrelation within the pitch range is determined
      if(*iR > *iRMax){
        iRMax = iR;   // Update the pointer to the maximum value
      }
    }

    // Calculate the lag corresponding to the maximum value of the autocorrelation
    unsigned int lag = iRMax - r.begin();

    float pot = 10 * log10(r[0]);

    // Print additional features for analysis (optional)
    // Based on that, implement a rule for unvoiced
    // Uncomment the following block and compile if needed
  #if 1
      if (r[0] > 0.0F)
        cout << pot << '\t' << r[1]/r[0] << '\t' << r[lag]/r[0] << endl;
  #endif

  /**
   * \DONE Se ha implementado la búsqueda del lag del valor máximo de la autocorrelación.
   * - Se han proporcionado opciones para establecer el valor mínimo del lag, ya sea:
   *    - El primer valor negativo de la autocorrelación.
   *    - El lag correspondiente al valor máximo del tono.
   * - En cualquier caso, el lag no debe exceder al del valor mínimo del tono.
   * - Se han mejorado los comentarios para explicar claramente el proceso de búsqueda del lag y los criterios para determinar segmentos no vocales.
   */
    
    // Check if the segment is unvoiced based on certain criteria
    if (unvoiced(zcr, r[1]/r[0], r[lag]/r[0], pot))
      return 0; // Segment is unvoiced
    else
      return (float) samplingFreq/(float) lag; // Return the pitch frequency
  }
}
