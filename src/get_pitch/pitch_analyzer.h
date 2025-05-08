/// @file

#ifndef PITCH_ANALYZER_H
#define PITCH_ANALYZER_H

#include <vector>
#include <algorithm>

#define HAMMING_A0 0.54


namespace upc {

    /// Minimum and maximum pitch values in Hertz
    constexpr float MIN_F0 = 20.0F;
    constexpr float MAX_F0 = 10000.0F;

    /**
     * @brief PitchAnalyzer class computes the pitch (in Hz) from a signal frame.
     */
    class PitchAnalyzer {
    public:
        /// Wndow type
        enum Window {
            RECT,       ///< Rectangular window
            HAMMING     ///< Hamming window
        };

        /**
         * @brief Sets the window type for signal processing.
         * 
         * @param type Window type.
         */
        void set_window(Window type);


    private:
        std::vector<float> window; ///< Precomputed window
        unsigned int frameLen;     ///< Length of frame (in samples)
        unsigned int samplingFreq; ///< Sampling rate (in samples per second)
        unsigned int npitch_min;   ///< Minimum value of pitch period (in samples)
        unsigned int npitch_max;   ///< Maximum value of pitch period (in samples)
        float r2maxth;             ///< Threshold for normalized maximum autocorrelation
        float r1r0th;              ///< Threshold for the ratio between autocorrelation indices 1 and 0
        float zcrth;               ///< Threshold for zero crossing rate (ZCR)
        float potth;               ///< Threshold for power 



        /**
         * @brief Computes autocorrelation from lag = 0 to r.size().
         * 
         * @param x Input signal.
         * @param r Autocorrelation vector.
         */
        void autocorrelation(const std::vector<float> &x, std::vector<float> &r) const;

        /**
         * @brief Computes the pitch (in Hz) of the input frame.
         * 
         * @param x Input frame.
         * @param zcr Zero crossing rate.
         * @return Pitch value (in Hz).
         */
        float compute_pitch(std::vector<float> &x, float zcr) const;

        /**
         * @brief Determines if the frame is unvoiced.
         * 
         * @param zcr Zero crossing rate.
         * @param r1norm Normalized autocorrelation at lag 1.
         * @param rmaxnorm Normalized maximum autocorrelation.
         * @return True if the frame is unvoiced, False otherwise.
         */
        bool unvoiced(float zcr, float r1norm, float rmaxnorm, float pot) const;


public:
    /**
     * @brief Constructor for PitchAnalyzer class.
     * 
     * @param fLen Frame length in samples.
     * @param sFreq Sampling rate in Hertz.
     * @param w Window type for signal processing (default is HAMMING).
     * @param min_F0 Minimum pitch value (default is MIN_F0).
     * @param max_F0 Maximum pitch value (default is MAX_F0).
     * @param r2maxth_ Threshold for normalized maximum autocorrelation.
     * @param r1r0th_ Threshold for the ratio between autocorrelation indices 1 and 0.
     * @param zcrth_ Threshold for zero crossing rate (ZCR).
     * @param potth_ Threshold for power.
     */
    PitchAnalyzer(unsigned int fLen, unsigned int sFreq, Window w = PitchAnalyzer::HAMMING,
                  float min_F0 = MIN_F0, float max_F0 = MAX_F0,
                  float r2maxth_ = 0, float r1r0th_ = 0, float zcrth_ = 0, float potth_ = 0)
        : frameLen(fLen), samplingFreq(sFreq), r2maxth(r2maxth_), r1r0th(r1r0th_), zcrth(zcrth_), potth(potth_) {
        set_f0_range(min_F0, max_F0);
        set_window(w);
    }

        /**
         * @brief Computes the pitch for the given vector.
         * 
         * @param _x Input vector.
         * @param zcr Zero crossing rate.
         * @return Pitch value (in Hz).
         */
        float operator()(const std::vector<float> &_x, float zcr) const {
            if (_x.size() != frameLen)
                return -1.0F;

            std::vector<float> x(_x); // Local copy of input frame
            return compute_pitch(x, zcr);
        }

        /**
         * @brief Computes the pitch for the given "C" vector.
         * 
         * @param pt Pointer to the input vector.
         * @param N Size of the input vector.
         * @param zcr Zero crossing rate.
         * @return Pitch value (in Hz).
         */
        float operator()(const float *pt, unsigned int N, float zcr) const {
            if (N != frameLen)
                return -1.0F;

            std::vector<float> x(N); // Local copy of input frame, size N
            std::copy(pt, pt + N, x.begin()); // Copy input values into local vector x
            return compute_pitch(x, zcr);
        }

        /**
         * @brief Computes the pitch for the given vector, expressed by the begin and end iterators.
         * 
         * @param begin Iterator pointing to the beginning of the input vector.
         * @param end Iterator pointing to the end of the input vector.
         * @param zcr Zero crossing rate.
         * @return Pitch value (in Hz).
         */
        float operator()(std::vector<float>::const_iterator begin, std::vector<float>::const_iterator end,
                         float zcr) const {
            if (end - begin != frameLen)
                return -1.0F;

            std::vector<float> x(end - begin); // Local copy of input frame, size N
            std::copy(begin, end, x.begin()); // Copy input values into local vector x
            return compute_pitch(x, zcr);
        }

        /**
         * @brief Sets the pitch range based on minimum and maximum pitch values (in Hz).
         * 
         * @param min_F0 Minimum pitch value (in Hz).
         * @param max_F0 Maximum pitch value (in Hz).
         */
        void set_f0_range(float min_F0, float max_F0);
    };
} // namespace upc

#endif