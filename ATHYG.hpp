#ifndef LOUIERIKSSON_ATHYG_HPP
#define LOUIERIKSSON_ATHYG_HPP

#include <array>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace LouiEriksson {
	
	/**
	 * @mainpage Version 1.0.0
	 * @class ATHYG
	 * @brief Contains utilities for deserialising astronomical data from the <a href="https://github.com/astronexus/ATHYG-Database/tree/main">ATHYG dataset</a>.
	 */
	struct ATHYG final {
	
	private:
		
		/**
		 * @fn static std::stringstream ReadAllText(const std::filesystem::path& _path)
		 * @brief Reads the contents of a text file.
		 * @param _path The path to the file.
		 * @return A std::stringstream containing the contents of the file.
		 * @throws std::runtime_error If the file path is invalid.
		 *
		 * This function reads the contents of a text file specified by the file path.
		 * It returns the contents of the file as a std::stringstream object.
		 *
		 * If the file does not exist or if the file cannot be opened, an exception is thrown.
		 */
		static std::stringstream ReadAllText(const std::filesystem::path& _path) {
		
			std::stringstream result;
			
			if (exists(_path)) {
			
				std::fstream fs;
				fs.open(_path, std::ios::in);
			
				if (fs.is_open()) {
					result << fs.rdbuf();
					fs.close();
				}
			}
			else {
				throw std::runtime_error("Invalid file path");
			}
		
			return result;
		}
		
		/**
		 * @brief Splits a string into a vector of substrings based on a delimiter.
		 *
		 * The provided string is split into substrings using the specified delimiter.
		 * The substrings are then stored in a vector and returned.
		 * If the template argument is std::string, the substrings are stored as std::string objects.
		 * If the template argument is std::string_view, the substrings are stored as std::string_view objects.
		 *
		 * @param[in] _string The string to split.
		 * @param[in] _divider The delimiter character to split the string by.
		 * @param[in] _capacity (optional) The initial capacity of the result vector. Defaults to 0.
		 *
		 * @tparam T The type of the resulting substrings. Must be either std::string or std::string_view.
		 *
		 * @return std::vector<T> A vector containing the substrings.
		 *
		 * @note The function assumes that the provided delimiter is a single character.
		 * @note If _capacity is specified, the internal vector capacity is set to that value.
		 * @note The last word of the string is always included in the result, even if it is not delimited.
		 * @note The function assumes that the template argument is std::string or std::string_view.
		 */
		template<typename T = std::string_view>
		static constexpr std::vector<T> Split(const std::string_view& _string, const char& _divider, const size_t& _capacity = 0U) {
			
			static_assert(std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>,
                  "Template argument must be std::string or std::string_view");
    
			std::vector<T> result;
			result.reserve(_capacity);
			
			// Boost library version of string splitting.
			// (https://www.boost.org/doc/libs/1_54_0/doc/html/string_algo/usage.html#idp166856528)
			std::string::size_type start = 0U;
			std::string::size_type end = 0U;
			
			while((end = _string.find(_divider, start)) != std::string::npos) {
			    result.emplace_back(_string.substr(start, end - start));
			    start = end + 1U;
			}
			
			// Last word is not delimited.
			result.emplace_back(_string.substr(start));
			
			return result;
		}
		
		/**
		 * @brief Converts an std::vector to an std::array of a specified size using move semantics.
		 *
		 * This function takes an std::vector and converts its elements to an std::array
		 * of a specified size. It throws an std::runtime_error if the size of the vector
		 * does not match the size of the array.
		 *
		 * @tparam _Tp The type of elements in the vector and array.
		 * @tparam _Nm The size of the array.
		 * @param[in,out] _vector The vector to be converted.
		 * @return The resulting array.
		 * @throws std::runtime_error if the size of the vector does not match the size of the array.
		 */
		template<typename _Tp, size_t _Nm>
		static constexpr std::array<_Tp, _Nm> ToArray(const std::vector<_Tp>&& _vector) {
			
			if (_vector.size() != _Nm) {
				
				Debug::Log("Vector -> Array size mismatch! (" + std::to_string(_vector.size()) + ", " + std::to_string(_Nm) + ")");
				
				throw std::runtime_error("Vector -> Array size mismatch!");
			}
			
			std::array<_Tp, _Nm> result;
			std::move(
			    _vector.begin(),
			    _vector.begin() + std::min(_vector.size(), result.size()),
			     result.begin()
			);
			
			return result;
		}
		
		/**
		 * @brief Attempts to parse a string into an optional value of type T.
		 *
		 * This function attempts to convert the given string into a value of type T.
		 * If the conversion is successful, the resulting value is wrapped in an optional object and returned.
		 * If the conversion fails, an empty optional object is returned.
		 *
		 * @tparam T The type of value to parse the string into.
		 * @param[in] _str The string to parse.
		 * @return An optional value of type T, containing the parsed value if the conversion is successful, or an empty optional if the conversion fails.
		 *
		 * @note The type T must provide a specialize definition of this function in order to support parsing for that type.
		 */
		template <typename T>
		static std::optional<T> TryParse(const std::string_view& _str) noexcept {
			
			T r;
			
		    char* e          = nullptr; // (end)
			constexpr auto b = 10;      // (base)
			
			try {
				
				     if constexpr (std::is_same_v<T, int                >) { r = static_cast<T>(std::strtol  (_str.data(), &e, b)); }
				else if constexpr (std::is_same_v<T, short              >) { r = static_cast<T>(std::strtol  (_str.data(), &e, b)); }
				else if constexpr (std::is_same_v<T, long               >) { r =                std::strtol  (_str.data(), &e, b ); }
				else if constexpr (std::is_same_v<T, long long          >) { r =                std::strtoll (_str.data(), &e, b ); }
				else if constexpr (std::is_same_v<T, unsigned int       >) { r = static_cast<T>(std::strtoul (_str.data(), &e, b)); }
				else if constexpr (std::is_same_v<T, unsigned short     >) { r = static_cast<T>(std::strtoul (_str.data(), &e, b)); }
				else if constexpr (std::is_same_v<T, unsigned long      >) { r =                std::strtoul (_str.data(), &e, b ); }
				else if constexpr (std::is_same_v<T, unsigned long long >) { r =                std::strtoull(_str.data(), &e, b ); }
				else if constexpr (std::is_same_v<T, float              >) { r =                std::strtof  (_str.data(), &e    ); }
				else if constexpr (std::is_same_v<T, double             >) { r =                std::strtod  (_str.data(), &e    ); }
				else if constexpr (std::is_same_v<T, long double        >) { r =                std::strtold (_str.data(), &e    ); }
				else if constexpr (std::is_same_v<T, char         > ||
				                   std::is_same_v<T, unsigned char> ||
								   std::is_same_v<T, signed char  > ||
						           std::is_same_v<T, char16_t     > ||
				                   std::is_same_v<T, char32_t     > ||
		                           std::is_same_v<T, wchar_t      >)
			    {
					if (!_str.empty()) {
			            r = static_cast<T>(_str[0U]);
					}
					else {
						e = const_cast<char*>(_str.data());
					}
				}
				else if constexpr (std::is_same_v<T, bool>) {
			        r = _str == "true" || _str == "True" || _str == "TRUE" || _str == "T" || _str == "1";
				}
				else {
					static_assert([]{ return false; }(), "No specialisation exists for parsing string to T");
				}
			}
			catch (const std::exception& err) {
				
				/* Shouldn't ever happen but catch anyway... */
				
				e = const_cast<char*>(_str.data());
				
				std::cerr << err.what() << std::endl;
			}
			
			return (e == _str.data()) ?
		        std::optional<T>(std::nullopt) :
				std::optional<T>(r);
		}
		
		template<>
		[[deprecated("Redundant operation: string_view to string conversion is not necessary.")]]
		inline std::optional<std::string> Utils::TryParse(const std::string_view& _str) noexcept {
			return std::string(_str);
		}
		
		template<>
		[[deprecated("Redundant operation: string_view to string_view conversion is not necessary.")]]
		inline std::optional<std::string_view> Utils::TryParse(const std::string_view& _str) noexcept {
			return _str;
		}
		
	public:
		
		/**
		 * @struct V1
		 * @brief Utility for deserialising version 1 of the <a href="https://github.com/astronexus/ATHYG-Database/tree/main">ATHYG dataset</a>.
		 *
		 * @see <a href="https://github.com/astronexus/ATHYG-Database/blob/main/version-info.md">ATHYG version info.</a>
		 */
		struct [[maybe_unused]] V1 final {
			
			[[maybe_unused]] const std::optional<const size_t>      id;
			[[maybe_unused]] const std::optional<const std::string> tyc;
			[[maybe_unused]] const std::optional<const size_t>      gaia;
			[[maybe_unused]] const std::optional<const size_t>      hyg;
			[[maybe_unused]] const std::optional<const size_t>      hip;
			[[maybe_unused]] const std::optional<const size_t>      hd;
			[[maybe_unused]] const std::optional<const size_t>      hr;
			[[maybe_unused]] const std::optional<const std::string> gl;
			[[maybe_unused]] const std::optional<const std::string> bayer;
			[[maybe_unused]] const std::optional<const std::string> flam;
			[[maybe_unused]] const std::optional<const std::string> con;
			[[maybe_unused]] const std::optional<const std::string> proper;
			[[maybe_unused]] const std::optional<const double>      ra;
			[[maybe_unused]] const std::optional<const double>      dec;
			[[maybe_unused]] const std::optional<const std::string> pos_src;
			[[maybe_unused]] const std::optional<const double>      dist;
			[[maybe_unused]] const std::optional<const double>      x0;
			[[maybe_unused]] const std::optional<const double>      y0;
			[[maybe_unused]] const std::optional<const double>      z0;
			[[maybe_unused]] const std::optional<const std::string> dist_src;
			[[maybe_unused]] const std::optional<const double>      mag;
			[[maybe_unused]] const std::optional<const double>      absmag;
			[[maybe_unused]] const std::optional<const std::string> mag_src;
			
			[[maybe_unused]] static constexpr size_t s_ElementCount { 23U };
			
			template <typename T>
			explicit V1(const std::array<T, s_ElementCount>& _values) noexcept :
				id      (TryParse<size_t>(_values[ 0U])),
				tyc     (                 _values[ 1U] ),
				gaia    (TryParse<size_t>(_values[ 2U])),
				hyg     (TryParse<size_t>(_values[ 3U])),
				hip     (TryParse<size_t>(_values[ 4U])),
				hd      (TryParse<size_t>(_values[ 5U])),
				hr      (TryParse<size_t>(_values[ 6U])),
				gl      (                 _values[ 7U] ),
				bayer   (                 _values[ 8U] ),
				flam    (                 _values[ 9U] ),
				con     (                 _values[10U] ),
				proper  (                 _values[11U] ),
				ra      (TryParse<double>(_values[12U])),
				dec     (TryParse<double>(_values[13U])),
				pos_src (                 _values[14U] ),
				dist    (TryParse<double>(_values[15U])),
				x0      (TryParse<double>(_values[16U])),
				y0      (TryParse<double>(_values[17U])),
				z0      (TryParse<double>(_values[18U])),
				dist_src(                 _values[19U] ),
				mag     (TryParse<double>(_values[20U])),
				absmag  (TryParse<double>(_values[21U])),
				mag_src (                 _values[22U]) {}
		};
		
		/**
		 * @struct V2
		 * @brief Utility for deserialising version 2 of the <a href="https://github.com/astronexus/ATHYG-Database/tree/main">ATHYG dataset</a>.
		 *
		 * @see <a href="https://github.com/astronexus/ATHYG-Database/blob/main/version-info.md">ATHYG version info.</a>
		 */
		struct [[maybe_unused]] V2 final {
		
			[[maybe_unused]] const std::optional<const size_t>      id;
			[[maybe_unused]] const std::optional<const std::string> tyc;
			[[maybe_unused]] const std::optional<const size_t>      gaia;
			[[maybe_unused]] const std::optional<const size_t>      hyg;
			[[maybe_unused]] const std::optional<const size_t>      hip;
			[[maybe_unused]] const std::optional<const size_t>      hd;
			[[maybe_unused]] const std::optional<const size_t>      hr;
			[[maybe_unused]] const std::optional<const std::string> gl;
			[[maybe_unused]] const std::optional<const std::string> bayer;
			[[maybe_unused]] const std::optional<const std::string> flam;
			[[maybe_unused]] const std::optional<const std::string> con;
			[[maybe_unused]] const std::optional<const std::string> proper;
			[[maybe_unused]] const std::optional<const double>      ra;
			[[maybe_unused]] const std::optional<const double>      dec;
			[[maybe_unused]] const std::optional<const std::string> pos_src;
			[[maybe_unused]] const std::optional<const double>      dist;
			[[maybe_unused]] const std::optional<const double>      x0;
			[[maybe_unused]] const std::optional<const double>      y0;
			[[maybe_unused]] const std::optional<const double>      z0;
			[[maybe_unused]] const std::optional<const std::string> dist_src;
			[[maybe_unused]] const std::optional<const double>      mag;
			[[maybe_unused]] const std::optional<const double>      absmag;
			[[maybe_unused]] const std::optional<const std::string> mag_src;
			[[maybe_unused]] const std::optional<const double>      rv;
			[[maybe_unused]] const std::optional<const std::string> rv_src;
			[[maybe_unused]] const std::optional<const double>      pm_ra;
			[[maybe_unused]] const std::optional<const double>      pm_dec;
			[[maybe_unused]] const std::optional<const double>      pm_src;
			[[maybe_unused]] const std::optional<const double>      vx;
			[[maybe_unused]] const std::optional<const double>      vy;
			[[maybe_unused]] const std::optional<const double>      vz;
			[[maybe_unused]] const std::optional<const double>      spect;
			[[maybe_unused]] const std::optional<const std::string> spect_src;
			
			[[maybe_unused]] static constexpr size_t s_ElementCount { 33U };
			
			template <typename T>
			explicit V2(const std::array<T, s_ElementCount>& _values) noexcept :
				id       (TryParse<size_t>(_values[ 0U])),
				tyc      (                 _values[ 1U] ),
				gaia     (TryParse<size_t>(_values[ 2U])),
				hyg      (TryParse<size_t>(_values[ 3U])),
				hip      (TryParse<size_t>(_values[ 4U])),
				hd       (TryParse<size_t>(_values[ 5U])),
				hr       (TryParse<size_t>(_values[ 6U])),
				gl       (                 _values[ 7U] ),
				bayer    (                 _values[ 8U] ),
				flam     (                 _values[ 9U] ),
				con      (                 _values[10U] ),
				proper   (                 _values[11U] ),
				ra       (TryParse<double>(_values[12U])),
				dec      (TryParse<double>(_values[13U])),
				pos_src  (                 _values[14U] ),
				dist     (TryParse<double>(_values[15U])),
				x0       (TryParse<double>(_values[16U])),
				y0       (TryParse<double>(_values[17U])),
				z0       (TryParse<double>(_values[18U])),
				dist_src (                 _values[19U] ),
				mag      (TryParse<double>(_values[20U])),
				absmag   (TryParse<double>(_values[21U])),
				mag_src  (                 _values[22U] ),
				rv       (TryParse<double>(_values[23U])),
				rv_src   (                 _values[24U] ),
				pm_ra    (TryParse<double>(_values[25U])),
				pm_dec   (TryParse<double>(_values[26U])),
				pm_src   (TryParse<double>(_values[27U])),
				vx       (TryParse<double>(_values[28U])),
				vy       (TryParse<double>(_values[29U])),
				vz       (TryParse<double>(_values[30U])),
				spect    (TryParse<double>(_values[31U])),
				spect_src(                 _values[32U]) {}
		};
		
		/**
		 * @struct V3
		 * @brief Utility for deserialising version 3 of the <a href="https://github.com/astronexus/ATHYG-Database/tree/main">ATHYG dataset</a>.
		 *
		 * @see <a href="https://github.com/astronexus/ATHYG-Database/blob/main/version-info.md">ATHYG version info.</a>
		 */
		struct [[maybe_unused]] V3 final {
			
			[[maybe_unused]] const std::optional<const size_t>      id;
			[[maybe_unused]] const std::optional<const std::string> tyc;
			[[maybe_unused]] const std::optional<const size_t>      gaia;
			[[maybe_unused]] const std::optional<const size_t>      hyg;
			[[maybe_unused]] const std::optional<const size_t>      hip;
			[[maybe_unused]] const std::optional<const size_t>      hd;
			[[maybe_unused]] const std::optional<const size_t>      hr;
			[[maybe_unused]] const std::optional<const std::string> gl;
			[[maybe_unused]] const std::optional<const std::string> bayer;
			[[maybe_unused]] const std::optional<const std::string> flam;
			[[maybe_unused]] const std::optional<const std::string> con;
			[[maybe_unused]] const std::optional<const std::string> proper;
			[[maybe_unused]] const std::optional<const double>      ra;
			[[maybe_unused]] const std::optional<const double>      dec;
			[[maybe_unused]] const std::optional<const std::string> pos_src;
			[[maybe_unused]] const std::optional<const double>      dist;
			[[maybe_unused]] const std::optional<const double>      x0;
			[[maybe_unused]] const std::optional<const double>      y0;
			[[maybe_unused]] const std::optional<const double>      z0;
			[[maybe_unused]] const std::optional<const std::string> dist_src;
			[[maybe_unused]] const std::optional<const double>      mag;
			[[maybe_unused]] const std::optional<const double>      absmag;
			[[maybe_unused]] const std::optional<const double>      ci;
			[[maybe_unused]] const std::optional<const std::string> mag_src;
			[[maybe_unused]] const std::optional<const double>      rv;
			[[maybe_unused]] const std::optional<const std::string> rv_src;
			[[maybe_unused]] const std::optional<const double>      pm_ra;
			[[maybe_unused]] const std::optional<const double>      pm_dec;
			[[maybe_unused]] const std::optional<const double>      pm_src;
			[[maybe_unused]] const std::optional<const double>      vx;
			[[maybe_unused]] const std::optional<const double>      vy;
			[[maybe_unused]] const std::optional<const double>      vz;
			[[maybe_unused]] const std::optional<const double>      spect;
			[[maybe_unused]] const std::optional<const std::string> spect_src;
			
			[[maybe_unused]] static constexpr size_t s_ElementCount { 34U };
			
			template <typename T>
			explicit V3(const std::array<T, s_ElementCount>& _values) noexcept :
				id       (TryParse<size_t>(_values[ 0U])),
				tyc      (                 _values[ 1U] ),
				gaia     (TryParse<size_t>(_values[ 2U])),
				hyg      (TryParse<size_t>(_values[ 3U])),
				hip      (TryParse<size_t>(_values[ 4U])),
				hd       (TryParse<size_t>(_values[ 5U])),
				hr       (TryParse<size_t>(_values[ 6U])),
				gl       (                 _values[ 7U] ),
				bayer    (                 _values[ 8U] ),
				flam     (                 _values[ 9U] ),
				con      (                 _values[10U] ),
				proper   (                 _values[11U] ),
				ra       (TryParse<double>(_values[12U])),
				dec      (TryParse<double>(_values[13U])),
				pos_src  (                 _values[14U] ),
				dist     (TryParse<double>(_values[15U])),
				x0       (TryParse<double>(_values[16U])),
				y0       (TryParse<double>(_values[17U])),
				z0       (TryParse<double>(_values[18U])),
				dist_src (                 _values[19U] ),
				mag      (TryParse<double>(_values[20U])),
				absmag   (TryParse<double>(_values[21U])),
				ci       (TryParse<double>(_values[22U])),
				mag_src  (                 _values[23U] ),
				rv       (TryParse<double>(_values[24U])),
				rv_src   (                 _values[25U] ),
				pm_ra    (TryParse<double>(_values[26U])),
				pm_dec   (TryParse<double>(_values[27U])),
				pm_src   (TryParse<double>(_values[28U])),
				vx       (TryParse<double>(_values[29U])),
				vy       (TryParse<double>(_values[30U])),
				vz       (TryParse<double>(_values[31U])),
				spect    (TryParse<double>(_values[32U])),
				spect_src(                 _values[33U] ) {}
		};
		
		/**
		 * @brief Load and parse ATHYG dataset files.
		 *
		 * This function loads and parses ATHYG CSV files.
		 * The template argument must be one of the ATHYG dataset versions:
		 * V1, V2, or V3.
		 *
		 * @param[in] _athyg_paths The paths to the ATHYG CSV file.
		 * @return A vector containing the deserialized data of type T.
		 * @throw std::runtime_error If the number of elements in a CSV line
		 * is not consistent with the ATHYG version.
		 * @throw std::runtime_error If the specified path is not valid.
		 *
		 * @tparam T The ATHYG dataset version (V1, V2, or V3).
		 */
		template <typename T>
		static std::vector<T> Load(const std::vector<std::filesystem::path>& _athyg_paths) {
			
			static_assert(std::is_same_v<T, ATHYG::V1> || std::is_same_v<T, ATHYG::V2> || std::is_same_v<T, ATHYG::V3>,
			        "Template argument must be an ATHYG version!");
			
			// Merged result
			std::vector<T> result;
			
			// Load and parse each file in parallel:
			for (const auto& path : _athyg_paths) {
			
				std::cout << "Parsing \"" + path.string() + "\"... " << std::flush;
			
				if (exists(path)) {
					
					auto csv = ReadAllText(path);
					
					std::string line;
					
					// Skip the header (first line) of the CSV.
					std::getline(csv, line);
					
					// Process CSV elements:
				    while (std::getline(csv, line)) {
						
						auto elements = Split(line, ',', T::s_ElementCount);
						
						// Discard elements beyond the count used by this ATHYG version.
						if (elements.size() > T::s_ElementCount) {
							elements.resize(T::s_ElementCount);
						}
						
						// Validate number of elements matches the count expected by the ATHYG version.
						if (elements.size() == T::s_ElementCount) {
							
							// Deserialise the star.
							result.emplace_back(T(ToArray<std::string_view, T::s_ElementCount>(std::move(elements))));
						}
						else {
							throw std::runtime_error("Number of elements not consistent with ATHYG version!");
						}
					}
				}
				else {
					throw std::runtime_error("Path is not valid.");
				}
				
				std::cout << "Done.\n";
			}
			
			return result;
		}
	};
	
} // LouiEriksson

#endif //LOUIERIKSSON_ATHYG_HPP