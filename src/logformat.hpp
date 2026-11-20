#ifndef LOGFORMAT_HPP
#define LOGFORMAT_HPP

#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <locale>
#include <ostream>
#include <sstream>
#include <string>

namespace log_output {

// Apply grouping only to log values, leaving VCF and histogram streams alone.
class GroupedNumbers : public std::numpunct<char> {
protected:
    char do_thousands_sep() const override { return ','; }
    std::string do_grouping() const override { return "\3"; }
};

inline const std::locale& number_locale() {
    static const std::locale value(std::locale::classic(), new GroupedNumbers);
    return value;
}

inline std::string count(std::size_t value) {
    std::ostringstream stream;
    stream.imbue(number_locale());
    stream << value;
    return stream.str();
}

inline std::string decimal(double value) {
    std::ostringstream stream;
    stream.imbue(number_locale());
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}

inline std::string filename(const std::string& path) {
    const std::size_t slash = path.find_last_of('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

inline std::string absolute_path(const std::string& path) {
    char* resolved = ::realpath(path.c_str(), nullptr);
    if (!resolved) return path;
    const std::string result(resolved);
    std::free(resolved);
    return result;
}

inline void section(std::ostream& out, const std::string& title) {
    const std::string rule(60, '=');
    out << "\n\n" << rule << '\n' << title << '\n' << rule << "\n\n";
}

inline void metric(std::ostream& out, const std::string& label, double value,
                   const std::string& unit = "") {
    std::ostringstream row;
    row << std::left << std::setw(26) << label
        << std::right << std::setw(10) << decimal(value) << unit << '\n';
    out << row.str();
}

} // namespace log_output

#endif // LOGFORMAT_HPP
