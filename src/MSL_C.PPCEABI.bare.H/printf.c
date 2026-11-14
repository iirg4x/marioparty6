/* Only the routines the game links are present. */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <wchar_io.h>
#include <ctype.h>

/* __num2dec's consumed layout: sign, exponent, then a counted digit sequence. */
typedef struct {
    signed char negative;
    short exponent;
    struct { unsigned char count; unsigned char digits[36]; } significand;
} DecimalValue;
typedef struct { unsigned char style; short digits; } DecimalRequest;
/* The bounded string writer tracks a pointer, capacity, and emitted length. */
typedef struct { char *destination; size_t capacity; size_t written; } StringTarget;
void *__FileWrite(void *, const char *, size_t);
void *__StringWrite(void *, const char *, size_t);

extern void __num2dec(const DecimalRequest*, double, DecimalValue*);

#define binary_width           64
#define binary_bytes          (binary_width / 8)
#define binary_max_exponent        LDBL_MAX_EXP
#define binary_precision       LDBL_MANT_DIG
#define binary_hidden_bit 1
#define binary_fraction_bits                                                 \
    (binary_precision - binary_hidden_bit)
#define binary_exponent_bits (binary_width - binary_fraction_bits - 1)

enum alignment {
    left_justified,
    right_justified,
    zero_filled
};

enum sign_policy { sign_negative, sign_explicit, sign_space };

enum argument_kind {
    arg_default,
    arg_char,
    arg_short,
    arg_long,
    arg_long_long,
    arg_long_double,
    arg_wide
};

typedef struct {
    unsigned char alignment;
    unsigned char sign_policy;
    unsigned char has_precision;
    unsigned char alternate;
    unsigned char argument_kind;
    unsigned char conversion;
    int width;
    int precision;
} FormatSpec;

/* Parse conversion flags, width, precision, and argument length. */
static const char* parse_format(const char *pattern, va_list *arguments, FormatSpec *format_spec) {
    FormatSpec parsed_spec;
    const char* text_cursor = pattern;
    int character;
    int found_flag;
    parsed_spec.alignment = right_justified;
    parsed_spec.sign_policy = sign_negative;
    parsed_spec.has_precision = 0;
    parsed_spec.alternate = 0;
    parsed_spec.argument_kind = arg_default;
    parsed_spec.width = 0;
    parsed_spec.precision = 0;

    if ((character = *++text_cursor) == '%') {
        parsed_spec.conversion = character;
        *format_spec = parsed_spec;
        return (text_cursor + 1);
    }

    for (;;) {
        found_flag = 1;

        switch (character) {
            case '-':
                parsed_spec.alignment = left_justified;
                break;
            case '+':
                parsed_spec.sign_policy = sign_explicit;
                break;
            case ' ':
                if (parsed_spec.sign_policy != sign_explicit) {
                    parsed_spec.sign_policy = sign_space;
                }
                break;
            case '#':
                parsed_spec.alternate = 1;
                break;
            case '0':
                if (parsed_spec.alignment != left_justified) {
                    parsed_spec.alignment = zero_filled;
                }
                break;
            default:
                found_flag = 0;
                break;
        }

        if (found_flag) {
            character = *++text_cursor;
        }
        else {
            break;
        }
    }

    if (character == '*') {
        if ((parsed_spec.width = va_arg(*arguments, int)) < 0) {
            parsed_spec.alignment = left_justified;
            parsed_spec.width = -parsed_spec.width;
        }

        character = *++text_cursor;
    }
    else {
        while (isdigit(character)) {
            parsed_spec.width = (parsed_spec.width * 10) + (character - '0');
            character = *++text_cursor;
        }
    }

    if (parsed_spec.width > 509) {
        parsed_spec.conversion = 255;
        *format_spec = parsed_spec;
        return (text_cursor + 1);
    }

    if (character == '.') {
        parsed_spec.has_precision = 1;

        if ((character = *++text_cursor) == '*') {
            if ((parsed_spec.precision = va_arg(*arguments, int)) < 0) {
                parsed_spec.has_precision = 0;
            }

            character = *++text_cursor;
        }
        else {
            while (isdigit(character)) {
                parsed_spec.precision = (parsed_spec.precision * 10) + (character - '0');
                character = *++text_cursor;
            }
        }
    }

    found_flag = 1;

    switch (character) {
        case 'h':
            parsed_spec.argument_kind = arg_short;

            if (text_cursor[1] == 'h') {
                parsed_spec.argument_kind = arg_char;
                character = *++text_cursor;
            }

            break;

        case 'l':
            parsed_spec.argument_kind = arg_long;

            if (text_cursor[1] == 'l') {
                parsed_spec.argument_kind = arg_long_long;
                character = *++text_cursor;
            }

            break;

        case 'L':
            parsed_spec.argument_kind = arg_long_double;
            break;
        default:
            found_flag = 0;
            break;
    }

    if (found_flag) {
        character = *++text_cursor;
    }

    parsed_spec.conversion = character;

    switch (character) {
        case 'd':
        case 'i':
        case 'u':
        case 'o':
        case 'x':
        case 'X':
            if (parsed_spec.argument_kind == arg_long_double) {
                parsed_spec.conversion = 255;
                break;
            }

            if (!parsed_spec.has_precision) {
                parsed_spec.precision = 1;
            }
            else if (parsed_spec.alignment == zero_filled) {
                parsed_spec.alignment = right_justified;
            }
            break;

        case 'f':
        case 'F':
            if (parsed_spec.argument_kind == arg_short || parsed_spec.argument_kind ==
                arg_long_long) {
                parsed_spec.conversion = 255;
                break;
            }

            if (!parsed_spec.has_precision) {
                parsed_spec.precision = 6;
            }
            break;

        case 'a':
        case 'A':
            if (!parsed_spec.has_precision) {
                parsed_spec.precision = 13;
            }

            if (parsed_spec.argument_kind == arg_short || parsed_spec.argument_kind ==
                arg_long_long || parsed_spec.argument_kind == arg_char) {
                parsed_spec.conversion = 255;
            }

            break;

        case 'g':
        case 'G':
            if (!parsed_spec.precision) {
                parsed_spec.precision = 1;
            }

        case 'e':
        case 'E':
            if (parsed_spec.argument_kind == arg_short || parsed_spec.argument_kind ==
                arg_long_long || parsed_spec.argument_kind == arg_char) {
                parsed_spec.conversion = 255;
                break;
            }

            if (!parsed_spec.has_precision) {
                parsed_spec.precision = 6;
            }
            break;

        case 'p':
            parsed_spec.conversion = 'x';
            parsed_spec.alternate = 1;
            parsed_spec.argument_kind = arg_long;
            parsed_spec.precision = 8;
            break;

        case 'c':
            if (parsed_spec.argument_kind == arg_long) {
                parsed_spec.argument_kind = arg_wide;
            }
            else {
                if (parsed_spec.has_precision || parsed_spec.argument_kind != arg_default) {
                    parsed_spec.conversion = 255;
                }
            }

            break;

        case 's':
            if (parsed_spec.argument_kind == arg_long) {
                parsed_spec.argument_kind = arg_wide;
            }
            else {
                if (parsed_spec.argument_kind != arg_default) {
                    parsed_spec.conversion = 255;
                }
            }

            break;

        case 'n':
            if (parsed_spec.argument_kind == arg_long_double) {
                    parsed_spec.conversion = 255;
            }

            break;

        default:
            parsed_spec.conversion = 255;
            break;
    }

    *format_spec = parsed_spec;
    return (text_cursor + 1);
}

/* Build a long integer conversion backward from the buffer end. */
static char* long2str(long number, char* workspace, FormatSpec format_spec)
{
    unsigned long magnitude, radix;
    char* cursor;
    int steps, digits;
    int is_negative    = 0;
    magnitude = number;

    cursor      = workspace;
    *--cursor   = 0;
    digits = 0;

    if (!number && !format_spec.precision
        && !(format_spec.alternate && format_spec.conversion == 'o')) {
        return cursor;
    }

    switch (format_spec.conversion) {
    case 'd':
    case 'i':
        radix = 10;

        if (number < 0) {
            magnitude = -magnitude;
            is_negative        = 1;
        }
        break;

    case 'o':
        radix                 = 8;
        format_spec.sign_policy = sign_negative;
        break;

    case 'u':
        radix                 = 10;
        format_spec.sign_policy = sign_negative;
        break;

    case 'x':
    case 'X':
        radix                 = 16;
        format_spec.sign_policy = sign_negative;
        break;
    }

    do {
        steps = magnitude % radix;
        magnitude /= radix;

        if (steps < 10) {
            steps += '0';
        } else {
            steps -= 10;

            if (format_spec.conversion == 'x') {
                steps += 'a';
            } else {
                steps += 'A';
            }
        }

        *--cursor = steps;
        ++digits;
    } while (magnitude != 0);

    if (radix == 8 && format_spec.alternate && *cursor != '0') {
        *--cursor = '0';
        ++digits;
    }

    if (format_spec.alignment == zero_filled) {
        format_spec.precision = format_spec.width;

        if (is_negative || format_spec.sign_policy != sign_negative)
            --format_spec.precision;

        if (radix == 16 && format_spec.alternate)
            format_spec.precision -= 2;
    }

    if (workspace - cursor + format_spec.precision > 509)
        return (0);

    while (digits < format_spec.precision) {
        *--cursor = '0';
        ++digits;
    }

    if (radix == 16 && format_spec.alternate) {
        *--cursor = format_spec.conversion;
        *--cursor = '0';
    }

    if (is_negative) {
        *--cursor = '-';
    } else if (format_spec.sign_policy == sign_explicit) {
        *--cursor = '+';
    } else if (format_spec.sign_policy == sign_space) {
        *--cursor = ' ';
    }

    return cursor;
}

/* Build a long long conversion using the requested radix and signs. */
static char* longlong2str(long long number, char* workspace, FormatSpec format_spec)
{
    unsigned long long magnitude, radix;
    char* cursor;
    int steps, digits;
    int is_negative    = 0;
    magnitude = number;
    is_negative        = 0;
    cursor            = workspace;
    *--cursor         = 0;
    digits       = 0;

    if (!number && !format_spec.precision
        && !(format_spec.alternate && format_spec.conversion == 'o')) {
        return cursor;
    }

    switch (format_spec.conversion) {
    case 'd':
    case 'i':
        radix = 10;

        if (number < 0) {
            magnitude = -magnitude;
            is_negative        = 1;
        }
        break;
    case 'o':
        radix              = 8;
        format_spec.sign_policy = sign_negative;
        break;
    case 'u':
        radix              = 10;
        format_spec.sign_policy = sign_negative;
        break;
    case 'x':
    case 'X':
        radix              = 16;
        format_spec.sign_policy = sign_negative;
        break;
    }

    do {
        steps = magnitude % radix;
        magnitude /= radix;

        if (steps < 10) {
            steps += '0';
        } else {
            steps -= 10;
            if (format_spec.conversion == 'x') {
                steps += 'a';
            } else {
                steps += 'A';
            }
        }

        *--cursor = steps;
        ++digits;
    } while (magnitude != 0);

    if (radix == 8 && format_spec.alternate && *cursor != '0') {
        *--cursor = '0';
        ++digits;
    }

    if (format_spec.alignment == zero_filled) {
        format_spec.precision = format_spec.width;

        if (is_negative || format_spec.sign_policy != sign_negative) {
            --format_spec.precision;
        }

        if (radix == 16 && format_spec.alternate) {
            format_spec.precision -= 2;
        }
    }

    if (workspace - cursor + format_spec.precision > 509) {
        return 0;
    }

    while (digits < format_spec.precision) {
        *--cursor = '0';
        ++digits;
    }

    if (radix == 16 && format_spec.alternate) {
        *--cursor = format_spec.conversion;
        *--cursor = '0';
    }

    if (is_negative) {
        *--cursor = '-';
    } else if (format_spec.sign_policy == sign_explicit) {
        *--cursor = '+';
    } else if (format_spec.sign_policy == sign_space) {
        *--cursor = ' ';
    }

    return cursor;
}

/* Render hexadecimal floating output, including infinities and NaNs. */
static char * double2hex(long double number, char * workspace, FormatSpec format_spec)  {
    char *cursor;
    unsigned char *scan_cursor;
    unsigned char nibble_value;
    char mantissa_byte;
    long double binary_value;
    long exponent;
    FormatSpec exponent_spec;
    int hex_digits;
    DecimalRequest decimal_request;
    DecimalValue decimal_value;

    cursor = workspace;
    binary_value = number;

    if (format_spec.precision > 509) {
        return 0;
    }

    decimal_request.style = (char) 0;
    decimal_request.digits = 32;
    __num2dec(&decimal_request, number, &decimal_value);

    if (*decimal_value.significand.digits == 'I') {
        if (*(short*) &binary_value & 0x8000) {
            cursor = workspace - 5;
            if (format_spec.conversion == 'A') strcpy(cursor, "-INF");
            else strcpy(cursor, "-inf");
        }
        else {
            cursor = workspace - 4;
            if (format_spec.conversion == 'A') strcpy(cursor, "INF");
            else strcpy(cursor, "inf");
        }

        return cursor;
    }
    if (*decimal_value.significand.digits == 'N') {
        if (*(unsigned char*) &number & 0x80) {
            cursor = workspace - 5;
            if (format_spec.conversion == 'A') strcpy(cursor, "-NAN");
            else strcpy(cursor, "-nan");
        }
        else {
            cursor = workspace - 4;
            if (format_spec.conversion == 'A') strcpy(cursor, "NAN");
            else strcpy(cursor, "nan");
        }

        return cursor;
    }

    exponent_spec.alignment = right_justified;
    exponent_spec.sign_policy = sign_explicit;
    exponent_spec.has_precision = 0;
    exponent_spec.alternate = 0;
    exponent_spec.argument_kind = arg_default;
    exponent_spec.width = 0;
    exponent_spec.precision = 1;
    exponent_spec.conversion = 'd';

    exponent = (short) ((*(short*) &binary_value & 0x7FFF) >> 4) - 1023;
    cursor = long2str(exponent, workspace, exponent_spec);
    if (format_spec.conversion == 'a')
        *--cursor = 'p';
    else
        *--cursor = 'P';
    scan_cursor = (unsigned char*) &number;

    for (hex_digits = format_spec.precision; hex_digits >= 1; hex_digits--) {
        nibble_value = *(scan_cursor + (hex_digits / 2 + 1));
        if (hex_digits % 2) {
            mantissa_byte = nibble_value & 0xF;
        } else {
            mantissa_byte = (nibble_value >> 4);
        }

        if (mantissa_byte < 10) {
            mantissa_byte += '0';
        }
        else
            if (format_spec.conversion == 'a') {
                mantissa_byte += 'a' - 10;
            }
            else {
                mantissa_byte += 'A' - 10;
            }

        *--cursor = mantissa_byte;
    }

    if (binary_hidden_bit){
        if (format_spec.precision || format_spec.alternate) {
            *--cursor = '.';
        }
        *--cursor = '1';
    }

    if (format_spec.conversion == 'a') {
        *--cursor = 'x';
    }
    else {
        *--cursor = 'X';
    }

    *--cursor = '0';

    if (*((short*) &binary_value) & 0x8000) {
        *--cursor = '-';
    }
    else if (format_spec.sign_policy == sign_explicit) {
        *--cursor = '+';
    }
    else if (format_spec.sign_policy == sign_space) {
        *--cursor = ' ';
    }

    return cursor;
}

/* Round the counted decimal digits, resolving exact ties to even. */
static void fn_800F3DB8(DecimalValue* decimal_value, int retained_digits)
{
    char character;
    char* cursor;
    int carry;

    if (retained_digits < 0) {
    zero_result:
        decimal_value->exponent        = 0;
        decimal_value->significand.count = 1;
        *decimal_value->significand.digits  = '0';
        return;
    }

    if (retained_digits >= decimal_value->significand.count) {
        return;
    }

    cursor = (char*)decimal_value->significand.digits + retained_digits + 1;
    character = *--cursor - '0';

    if (character == 5) {
        char* scan_cursor =
            (char*)decimal_value->significand.digits + decimal_value->significand.count;

        while (--scan_cursor > cursor && *scan_cursor == '0')
            ;
        carry = (scan_cursor == cursor) ? cursor[-1] & 1 : 1;
    } else {
        carry = (character > 5);
    }

    while (retained_digits != 0) {
        character = *--cursor - '0' + carry;

        if ((carry = (character > 9)) != 0 || character == 0) {
            --retained_digits;
        } else {
            *cursor = character + '0';
            break;
        }
    }

    if (carry != 0) {
        decimal_value->exponent += 1;
        decimal_value->significand.count = 1;
        *decimal_value->significand.digits  = '1';
        return;
    } else if (retained_digits == 0) {
        goto zero_result;
    }

    decimal_value->significand.count = retained_digits;
}

/* Render decimal floating output in fixed, exponential, or general form. */
static char* float2str(long double number, char *workspace, FormatSpec format_spec) {
    DecimalValue decimal_value;
    DecimalRequest decimal_request;
    char* cursor;
    char* scan_cursor;
    int steps, digits, sign_value;
    int integer_digits, fraction_digits;

    if (format_spec.precision > 509) {
        return 0;
    }

    decimal_request.style = 0;
    decimal_request.digits = 32;
    __num2dec(&decimal_request, number, &decimal_value);
    cursor = (char*)decimal_value.significand.digits + decimal_value.significand.count;

    while (decimal_value.significand.count > 1 && *--cursor == '0') {
        --decimal_value.significand.count;
        ++decimal_value.exponent;
    }

    switch (*decimal_value.significand.digits) {
        case '0':
            decimal_value.exponent = 0;
            break;
        case 'I':
            if (number < 0) {
                cursor = workspace - 5;

                if (isupper(format_spec.conversion)) {
                    strcpy(cursor, "-INF");
                }
                else {
                    strcpy(cursor, "-inf");
                }
            }
            else {
                cursor = workspace - 4;
                if (isupper(format_spec.conversion)) {
                    strcpy(cursor, "INF");
                }
                else {
                    strcpy(cursor, "inf");
                }
            }

            return cursor;

        case 'N':
            if (decimal_value.negative) {
                cursor = workspace - 5;

                if (isupper(format_spec.conversion)) {
                    strcpy(cursor, "-NAN");
                }
                else {
                    strcpy(cursor, "-nan");
                }
            }
            else {
                cursor = workspace - 4;
                if (isupper(format_spec.conversion)) {
                    strcpy(cursor, "NAN");
                }
                else {
                    strcpy(cursor, "nan");
                }
            }

            return cursor;
    }

    decimal_value.exponent += decimal_value.significand.count - 1;
    cursor = workspace;
    *--cursor = 0;

    switch (format_spec.conversion)
    {
        case 'g':
        case 'G':

            if (decimal_value.significand.count > format_spec.precision) {
                fn_800F3DB8(&decimal_value, format_spec.precision);
            }

            if (decimal_value.exponent < -4 || decimal_value.exponent >= format_spec.precision)
            {
                if (format_spec.alternate) {
                    --format_spec.precision;
                }
                else {
                    format_spec.precision = decimal_value.significand.count - 1;
                }

                if (format_spec.conversion == 'g') {
                    format_spec.conversion = 'e';
                }
                else {
                    format_spec.conversion = 'E';
                }

                goto exponential_form;
            }

            if (format_spec.alternate) {
                format_spec.precision -= decimal_value.exponent + 1;
            }
            else {
                if ((format_spec.precision = decimal_value.significand.count -
                    (decimal_value.exponent + 1)) < 0) {
                    format_spec.precision = 0;
                }
            }

            goto fixed_form;

        case 'e':
        case 'E':
        exponential_form:

            if (decimal_value.significand.count > format_spec.precision + 1) {
                fn_800F3DB8(&decimal_value, format_spec.precision + 1);
            }

            steps = decimal_value.exponent;
            sign_value = '+';

            if (steps < 0) {
                steps = -steps;
                sign_value = '-';
            }

            for (digits = 0; steps || digits < 2; ++digits) {
                *--cursor = steps % 10 + '0';
                steps /= 10;
            }

            *--cursor = sign_value;
            *--cursor = format_spec.conversion;

            if (workspace - cursor + format_spec.precision > 509) {
                return 0;
            }

            if (decimal_value.significand.count < format_spec.precision + 1) {
                for (steps = format_spec.precision + 1 - decimal_value.significand.count + 1;
                    --steps;) {
                    *--cursor = '0';
                }
            }

            for (steps = decimal_value.significand.count, scan_cursor =
                (char*)decimal_value.significand.digits + decimal_value.significand.count;
                --steps;) {
                *--cursor = *--scan_cursor;
            }

            if (format_spec.precision || format_spec.alternate) {
                *--cursor = '.';
            }

            *--cursor = *decimal_value.significand.digits;

            if (decimal_value.negative)
                *--cursor = '-';
            else if (format_spec.sign_policy == sign_explicit)
                *--cursor = '+';
            else if (format_spec.sign_policy == sign_space)
                *--cursor = ' ';

            break;

        case 'f':
        case 'F':
        fixed_form:

            if ((fraction_digits = -decimal_value.exponent + decimal_value.significand.count -
                1) < 0)
                fraction_digits = 0;

            if (fraction_digits > format_spec.precision) {
                fn_800F3DB8(&decimal_value, decimal_value.significand.count - (fraction_digits -
                    format_spec.precision));

                if ((fraction_digits = -decimal_value.exponent + decimal_value.significand.count -
                    1) < 0)
                    fraction_digits = 0;
            }

            if ((integer_digits = decimal_value.exponent + 1) < 0)
                integer_digits = 0;

            if (integer_digits + fraction_digits > 509)
                return 0;

            scan_cursor = (char *) decimal_value.significand.digits +
                decimal_value.significand.count;

            for (digits = 0; digits < (format_spec.precision - fraction_digits); ++digits)
                *--cursor = '0';

            for (digits = 0; digits < fraction_digits && digits < decimal_value.significand.count;
                ++digits)
                *--cursor = *--scan_cursor;

            for (; digits < fraction_digits; ++digits)
                *--cursor = '0';

            if (format_spec.precision || format_spec.alternate)
                *--cursor = '.';

            if (integer_digits) {
                for (digits = 0; digits < integer_digits - decimal_value.significand.count;
                    ++digits) {
                    *--cursor = '0';
                }

                for (; digits < integer_digits; ++digits) {
                    *--cursor = *--scan_cursor;
                }
            }
            else {
                *--cursor = '0';
            }

            if (decimal_value.negative) {
                *--cursor = '-';
            }
            else if (format_spec.sign_policy == sign_explicit) {
                *--cursor = '+';
            }
            else if (format_spec.sign_policy == sign_space) {
                *--cursor = ' ';
            }

            break;
    }

    return cursor;
}

/* Convert each format directive and send its bytes to the writer. */
static int __pformatter(void *(*write_bytes)(void *, const char *, size_t), void *write_context,
    const char * pattern, va_list arguments) {
    int segment_length, total_written, width;
    const char* pattern_cursor;
    const char* conversion_start;
    FormatSpec format_spec;
    long integer_value;
    long long wide_integer;
    long double floating_value;
    char workspace[512];
    char* rendered_text;
    char* terminator;
    char fill_byte = ' ';

    pattern_cursor = pattern;
    total_written = 0;

    while (*pattern_cursor) {
        if (!(conversion_start = strchr(pattern_cursor, '%'))) {
            segment_length = strlen(pattern_cursor);
            total_written += segment_length;

            if (segment_length && !(*write_bytes)(write_context, pattern_cursor, segment_length)) {
                return -1;
            }

            break;
        }

        segment_length = conversion_start - pattern_cursor;
        total_written += segment_length;

        if (segment_length && !(*write_bytes)(write_context, pattern_cursor, segment_length)) {
            return -1;
        }

        pattern_cursor = conversion_start;
        pattern_cursor = parse_format(pattern_cursor, (va_list*)arguments, &format_spec);

        switch (format_spec.conversion) {
            case 'd':
            case 'i':
                if (format_spec.argument_kind == arg_long) {
                    integer_value = va_arg(arguments, long);
                }
                else if (format_spec.argument_kind == arg_long_long) {
                    wide_integer = va_arg(arguments, long long);
                }
                else {
                    integer_value = va_arg(arguments, int);
                }

                if (format_spec.argument_kind == arg_short) {
                    integer_value = (short)integer_value;
                }

                if (format_spec.argument_kind == arg_char) {
                    integer_value = (signed char)integer_value;
                }

                if (format_spec.argument_kind == arg_long_long) {
                    if (!(rendered_text = longlong2str(wide_integer, workspace + 512,
                        format_spec))) {
                        goto invalid_conversion;
                    }
                }
                else {
                    if (!(rendered_text = long2str(integer_value, workspace + 512, format_spec))) {
                        goto invalid_conversion;
                    }
                }

                segment_length = workspace + 512 - 1 - rendered_text;
                break;

            case 'o':
            case 'u':
            case 'x':
            case 'X':
                if (format_spec.argument_kind == arg_long) {
                    integer_value = va_arg(arguments, unsigned long);
                }
                else if (format_spec.argument_kind == arg_long_long) {
                    wide_integer = va_arg(arguments, long long);
                }
                else {
                    integer_value = va_arg(arguments, unsigned int);
                }

                if (format_spec.argument_kind == arg_short) {
                    integer_value = (unsigned short)integer_value;
                }

                if (format_spec.argument_kind == arg_char) {
                    integer_value = (unsigned char)integer_value;
                }

                if (format_spec.argument_kind == arg_long_long) {
                    if (!(rendered_text = longlong2str(wide_integer, workspace + 512,
                        format_spec))) {
                        goto invalid_conversion;
                    }
                }
                else {
                    if (!(rendered_text = long2str(integer_value, workspace + 512, format_spec))) {
                        goto invalid_conversion;
                    }
                }

                segment_length = workspace + 512 - 1 - rendered_text;
                break;

            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                if (format_spec.argument_kind == arg_long_double) {
                    floating_value = va_arg(arguments, long double);
                }
                else {
                    floating_value = va_arg(arguments, double);
                }

                if (!(rendered_text = float2str(floating_value, workspace + 512, format_spec))) {
                    goto invalid_conversion;
                }

                segment_length = workspace + 512 - 1 - rendered_text;
                break;

            case 'a':
            case 'A':
                if (format_spec.argument_kind == arg_long_double) {
                    floating_value = va_arg(arguments, long double);
                }
                else {
                    floating_value = va_arg(arguments, double);
                }

                if (!(rendered_text = double2hex(floating_value, workspace + 512, format_spec))) {
                    goto invalid_conversion;
                }

                segment_length = workspace + 512 - 1 - rendered_text;
                break;

            case 's':
                if (format_spec.argument_kind == arg_wide) {
                    wchar_t* wide_text = va_arg(arguments, wchar_t*);

                    if(wide_text == NULL){
                        wide_text = L"";
                    }

                    if ((segment_length = wcstombs(workspace, wide_text, sizeof(workspace))) < 0) {
                        goto invalid_conversion;
                    }

                    rendered_text = &workspace[0];
                }
                else {
                    rendered_text = va_arg(arguments, char *);
                }

                if (rendered_text == NULL) {
                    rendered_text = "";
                }

                if (format_spec.alternate) {
                    segment_length = (unsigned char)*rendered_text++;

                    if (format_spec.has_precision && segment_length > format_spec.precision) {
                        segment_length = format_spec.precision;
                    }
                }
                else if (format_spec.has_precision) {
                    segment_length = format_spec.precision;

                    if ((terminator = (char*)memchr((unsigned char*)rendered_text, 0,
                        segment_length)) != 0) {
                        segment_length = terminator - rendered_text;
                    }
                }
                else {
                    segment_length = strlen(rendered_text);
                }

                break;

            case 'n':
                rendered_text = va_arg(arguments, char *);

                switch (format_spec.argument_kind) {
                    case arg_default:
                        *(int*)rendered_text = total_written;
                        break;
                    case arg_short:
                        *(short*)rendered_text = total_written;
                        break;
                    case arg_long:
                        *(long*)rendered_text = total_written;
                        break;
                    case arg_long_long:
                        *(long long*)rendered_text = total_written;
                        break;
                }

                continue;

            case 'c':
                rendered_text = workspace;
                *rendered_text = va_arg(arguments, int);
                segment_length = 1;
                break;

            case '%':
                rendered_text = workspace;
                *rendered_text = '%';
                segment_length = 1;
                break;

            case 255:
            default:
                invalid_conversion:
                    segment_length = strlen(conversion_start);
                    total_written += segment_length;

                    if (segment_length && !(*write_bytes)(write_context, conversion_start,
                        segment_length)) {
                        return -1;
                    }

                    return total_written;
                    break;
        }

        width = segment_length;

        if (format_spec.alignment != left_justified) {
            fill_byte = (format_spec.alignment == zero_filled) ? '0' : ' ';

            if (((*rendered_text == '+') || (*rendered_text == '-') || (*rendered_text == ' ')) &&
                (fill_byte == '0')) {
                if ((*write_bytes)(write_context, rendered_text, 1) == 0) {
                    return -1;
                }

                ++rendered_text;
                segment_length--;
            }

            while (width < format_spec.width) {
                if ((*write_bytes)(write_context, &fill_byte, 1) == 0) {
                    return -1;
                }

                ++width;
            }
        }

        if (segment_length && !(*write_bytes)(write_context, rendered_text, segment_length)) {
            return -1;
        }

        if (format_spec.alignment == left_justified) {
            while (width < format_spec.width) {
                char blank = ' ';

                if ((*write_bytes)(write_context, &blank, 1) == 0) {
                    return -1;
                }

                ++width;
            }
        }

        total_written += width;
    }

    return total_written;
}

/* Return the stream context only when every requested byte was written. */
void* __FileWrite(void* stream_context, const char* source_bytes, size_t requested_count)
{
    return (fwrite(source_bytes, 1, requested_count, (FILE*)stream_context) ==
        requested_count ? stream_context : 0);
}

/* Copy the portion that fits and advance the bounded target count. */
void* __StringWrite(void* string_context, const char* source_bytes, size_t requested_count)
{
    size_t copy_count;
    StringTarget* target = (StringTarget*)string_context;

    copy_count = ((target->written + requested_count) <= target->capacity)
                ? requested_count
                : target->capacity - target->written;
    memcpy(target->destination + target->written, source_bytes, copy_count);
    target->written += copy_count;
    return (void*) 1;
}

/* Format variadic arguments to byte-oriented standard output. */
int printf(const char* format_spec, ...)
{
    int result;

    if (fwide(stdout, -1) >= 0) {
        return -1;
    }

    {
        va_list arguments;
        va_start(arguments, format_spec);
        result = __pformatter(&__FileWrite, (void*)stdout, format_spec, arguments);
    }

    return result;
}

/* Format variadic arguments to a byte-oriented stream. */
int fprintf(FILE *stream, const char *pattern, ...)
{
    int result;
    va_list arguments;
    if (fwide(stream, -1) >= 0) {
        return -1;
    }
    va_start(arguments, pattern);
    result = __pformatter(&__FileWrite, (void *)stream, pattern, arguments);
    return result;
}

/* Format an existing argument list to byte-oriented standard output. */
int vprintf(const char* format_spec, va_list arguments)
{
    int result;

    if (fwide(stdout, -1) >= 0) {
        return -1;
    }

    result = __pformatter(&__FileWrite, (void*)stdout, format_spec, arguments);
    return result;
}

/* Format a bounded string and terminate the portion within its capacity. */
int vsnprintf(char* text_cursor, size_t steps, const char* format_spec, va_list arguments)
{
    int result;
    StringTarget target;
    target.destination      = text_cursor;
    target.capacity = steps;
    target.written = 0;

    result = __pformatter(&__StringWrite, &target, format_spec, arguments);

    if (text_cursor) {
        text_cursor[(result < steps) ? result : steps - 1] = '\0';
    }

    return result;
}

/* Format an argument list with the unbounded destination convention. */
int vsprintf(char *text_cursor, const char *format_spec, va_list arguments) {
    return vsnprintf(text_cursor, 0xFFFFFFFF /* No output limit. */, format_spec, arguments);
}

/* Format variadic arguments with the unbounded destination convention. */
int sprintf(char* text_cursor, const char* format_spec, ...)
{
    va_list arguments;
    va_start(arguments, format_spec);
    return vsnprintf(text_cursor, 0xFFFFFFFF /* No output limit. */, format_spec, arguments);
}
