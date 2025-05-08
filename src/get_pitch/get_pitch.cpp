/// @file

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <errno.h>
#include <vector>

#include "wavfile_mono.h"
#include "pitch_analyzer.h"
#include "docopt.h"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Estimator 

Usage:
    get_pitch [options] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    -m REAL, --medfilt=REAL  ODD Number to set Median Filter length. [default: 1]
    -c REAL, --clipmult=REAL  Clipping max multiplier [default: 0.0073]
    -r REAL, --r2maxth=REAL  Normalized Autocorrelation 2nd Max Threshold.[default: 0.39]
    -1 REAL, --r1r0th=REAL  Autocorrelation r[1]/r[0] relation Threshold. [default: 0.545]
    -z REAL, --zcrth=REAL  ZCR Threshold. [default: 2500]
    -p REAL, --potth=REAL  Threshold for power. [default: -52.1]


    -h, --help  Show this screen
    --version   Show the version of the project

Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the estimation:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";
float abs_f(float value){
  if (value < 0.0)
    return -1.0*value;
  return value;
}

int main(int argc, const char *argv[]) {
	/// \TODO 
	///  Modify the program syntax and the call to **docopt()** in order to
	///  add options and arguments to the program.
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},	// array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string

  // Parse command-line arguments
	std::string input_wav = args["<input-wav>"].asString();
	std::string output_txt = args["<output-txt>"].asString();
  float r2maxth = stof(args["--r2maxth"].asString());
  float clipmult = stof(args["--clipmult"].asString());
  float r1r0th = stof(args["--r1r0th"].asString());
  float zcrth = stof(args["--zcrth"].asString());
  float medfilt = stof(args["--medfilt"].asString());
  float potth = stof(args["--potth"].asString());


  /**
     * \DONE Se ha modificado la sintaxis del programa y la llamada a **docopt()** para
     *  agregar opciones y argumentos al programa.
     * - Se han añadido variables adicionales para almacenar los valores de las opciones adicionales
     *   proporcionadas por el usuario, como `r2maxth`, `clipmult`, `r1r0th`, `zcrth`, `potth` y `medfilt`.
     * - Estas variables son utilizadas posteriormente en el programa para ajustar su comportamiento
     *   según las preferencias del usuario.
     */
  
  // Read input sound file
  unsigned int rate;
  vector<float> x;
  if (readwav_mono(input_wav, rate, x) != 0) {
    cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
    return -2;
  }

  int n_len = rate * FRAME_LEN;
  int n_shift = rate * FRAME_SHIFT;

  // Define analyzer
  PitchAnalyzer analyzer(n_len, rate, PitchAnalyzer::RECT, 50, 500, r2maxth, r1r0th, zcrth, potth);

  /// \TODO
  /// Preprocess the input signal in order to ease pitch estimation. For instance,
  /// central-clipping or low pass filtering may be used.
  std::vector<float>::iterator iX, it;

  // Compute the clipping level (Cl) based on the maximum absolute value in the input signal
  float Cl = -1.0;
  for(iX = x.begin(); iX < x.end(); ++iX){    
    Cl = std::max(Cl, abs_f(*iX)); // Update Cl to the maximum absolute value found in the input signal
  }
  Cl = clipmult * Cl; // Scale Cl using the clip multiplier

  // Iterate for each frame and save values in f0 vector
  vector<float> f0; // Vector to store the pitch values
  float f, aux, prev, act, zcr=0; // Variables for pitch analysis and zero crossing rate (ZCR) computation
  float cte = rate / (2 * (n_len - 1)); // Constant used to compute ZCR
  for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
    // Preprocess each frame: compute ZCR and perform central-clipping
    prev=0; aux=0;
    for(it = iX; it < iX + n_len; ++it){  //COMPUTE ZCR and Clipping:
      act = *it;
      if((act * prev) < 0){ aux++;} // Count zero crossings
      prev = act;

      if(abs(act) < Cl){ *it = 0;} // Apply central-clipping
      else *it = *it + Cl * ((act < 0) - (act > 0));
    }
    
    zcr = aux * cte; // Compute ZCR for the frame
    // Perform pitch analysis on the preprocessed frame and store the result in f0
    f = analyzer(iX, iX + n_len, zcr); // Analyze pitch with ZCR information
    f0.push_back(f); // Store the pitch value in the f0 vector
  }

  // JUST ODD NUMBERS  
  int F_size = medfilt;  
  vector<float> filter; 

  /**
   * \DONE Se ha implementado el preprocesamiento de la señal de entrada para facilitar la estimación del tono.
   * - Se calcula el nivel de corte central (Cl) basado en el valor absoluto máximo en la señal de entrada.
   * - Se itera sobre cada trama y se preprocesa para calcular la tasa de cruce por cero (ZCR) y aplicar el corte central.
   * - Se realiza el análisis de tono en cada trama preprocesada y se almacenan los valores en el vector f0.
   * - Se ha añadido una explicación del proceso de preprocesamiento y análisis de tono, así como el uso de ZCR para mejorar la precisión.
   */


  /// \TODO
  /// Postprocess the estimation in order to suppress errors. For instance, a median filter
  /// or time-warping may be used.
  for(iX = f0.begin(); iX < f0.end() - (F_size - 1); ++iX){    
    for(int i = 0; i<F_size; i++)      
      filter.push_back(*(iX+i)); // Populate the filter with elements from the f0 vector

    int k, l;
    // Sort the filter elements in ascending order
    for(k = 0; k < F_size-1; k++){      // Sort:
      for(l = 0; l < F_size-k-1; l++){
        if (filter[l] > filter[l+1]){        
          aux = filter[l];        
          filter[l] = filter[l+1]; 
          filter[l+1] = aux;      
        }    
      }   
    }
    // Get the median value from the sorted filter
    f0[iX - f0.begin()] = filter[F_size/2]; // Replace the original pitch value with the median
    filter.clear(); // Clear the filter for the next iteration  
  } 

  // Write f0 contour into the output file
  ofstream os(output_txt);
  if (!os.good()) {
    cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
    return -3;
  }

  os << 0 << '\n'; // Pitch at t=0
  for (iX = f0.begin(); iX != f0.end(); ++iX) 
    os << *iX << '\n'; // Write pitch values to the output file
  os << 0 << '\n'; // Pitch at t=Dur

  /**
   * \DONE Se ha implementado el postprocesamiento de la estimación para suprimir errores.
   * - Se ha utilizado un filtro mediano para suavizar la estimación y reducir los errores.
   * - Se ha añadido una explicación detallada del proceso de postprocesamiento y la técnica utilizada.
   */
  
  return 0;

}