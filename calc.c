// Shashank Sagar Narwa - Student ID: 251ADB118
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: none

#include <ctype.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define NAME  "Shashank"
#define LAST  "Narwa"
#define ID    "251ADB118"

static char *text;      // whole file in memory, ends with '\0' so loops stop safely
static long length;     // file size
static long pos;        // index of current character; position in the rules = pos + 1
static long error_pos;  // 0 = no error, else position of the FIRST error

typedef struct { int is_float; long long i; double d; } Number;
static Number I(long long v) { Number n = {0, v, 0.0}; return n; }
static Number F(double v)    { Number n = {1, 0, v};   return n; }

static void fail(long where) { if (!error_pos) error_pos = where; }
static void skip(void) { while (text[pos] == ' ' || text[pos] == '\t') pos++; }

// A float that is infinity or NaN is an error at the operator.
static Number float_ok(double v, long op) {
    if (!isfinite(v)) { fail(op); return I(0); }
    return F(v);
}

static Number expr(void);

// number = digits [ '.' digits ]. A '.' without a digit after it is left
// alone, so the caller reports it as the error ("5.", ".5", "1.5e3").
static Number number(void) {
    long start = pos;
    long long v = 0;
    while (isdigit((unsigned char)text[pos])) v = v * 10 + (text[pos++] - '0');
    if (text[pos] == '.' && isdigit((unsigned char)text[pos + 1])) {
        pos++;
        while (isdigit((unsigned char)text[pos])) pos++;
        char saved = text[pos];     // strtod needs a '\0' at the end of the number,
        text[pos] = '\0';           // so put one there for a moment (no copying)
        double d = strtod(text + start, NULL);
        text[pos] = saved;
        return float_ok(d, start + 1);
    }
    return I(v);
}

// op is one of + - * / and 'p' for **. op_pos = operator position,
// right_pos = where the right operand starts. Integer math only if both
// operands are integers, otherwise everything is done in double.
static Number apply(char op, Number a, Number b, long op_pos, long right_pos) {
    if (!a.is_float && !b.is_float) {
        if (op == '+') return I(a.i + b.i);
        if (op == '-') return I(a.i - b.i);
        if (op == '*') return I(a.i * b.i);
        if (op == '/') {
            if (b.i == 0) { fail(right_pos); return I(0); }
            return I(a.i / b.i);            // C truncates toward zero
        }
        if (b.i < 0) { fail(right_pos); return I(0); }   // int ** negative int
        long long r = 1, base = a.i;
        for (long long e = b.i; e > 0; e /= 2) {         // repeated squaring
            if (e % 2) r *= base;
            if (e > 1) base *= base;
        }
        return I(r);
    }
    double x = a.is_float ? a.d : (double)a.i;
    double y = b.is_float ? b.d : (double)b.i;
    if (op == '+') return float_ok(x + y, op_pos);
    if (op == '-') return float_ok(x - y, op_pos);
    if (op == '*') return float_ok(x * y, op_pos);
    if (op == 'p') return float_ok(pow(x, y), op_pos);
    if (y == 0.0) { fail(right_pos); return I(0); }
    return float_ok(x / y, op_pos);
}

// Grammar, one function per precedence level. Each level only calls the
// next tighter level for its operands, so ** binds tighter than * /,
// which bind tighter than + -.
//   expr    = term  { (+|-) term }
//   term    = power { (*|/) power }
//   power   = primary [ ** power ]
//   primary = NUMBER | ( expr )

// Any problem here is reported at the current character (the token we could
// not continue with): bad character, operator instead of a number, the ')' in
// "()", or the '\n' / end of file when an operand is missing.
static Number primary(void) {
    skip();
    if (isdigit((unsigned char)text[pos])) return number();
    if (text[pos] == '(') {
        pos++;
        Number v = expr();
        if (error_pos) return v;
        skip();
        if (text[pos] == ')') { pos++; return v; }
    }
    fail(pos + 1);
    return I(0);
}

// power calls itself for the exponent, so 2 ** 3 ** 2 = 2 ** (3 ** 2).
static Number power(void) {
    Number a = primary();
    skip();
    if (error_pos || text[pos] != '*' || text[pos + 1] != '*') return a;
    long op_pos = pos + 1;
    pos += 2;
    skip();
    long right_pos = pos + 1;       // remember it now, pos moves on
    Number b = power();
    if (error_pos) return b;
    return apply('p', a, b, op_pos, right_pos);
}

// term and expr use loops, so 8 - 3 - 2 = (8 - 3) - 2 (left to right).
static Number term(void) {
    Number a = power();
    for (;;) {
        skip();
        char op = text[pos];
        if (error_pos || (op != '*' && op != '/')) return a;
        long op_pos = pos + 1;
        pos++;
        skip();
        long right_pos = pos + 1;
        Number b = power();
        if (error_pos) return b;
        a = apply(op, a, b, op_pos, right_pos);
    }
}

static Number expr(void) {
    Number a = term();
    for (;;) {
        skip();
        char op = text[pos];
        if (error_pos || (op != '+' && op != '-')) return a;
        long op_pos = pos + 1;
        pos++;
        skip();
        long right_pos = pos + 1;
        Number b = term();
        if (error_pos) return b;
        a = apply(op, a, b, op_pos, right_pos);
    }
}

// Evaluate every line; write each result to out. Stops at the first error.
static void run(FILE *out) {
    pos = 0;
    error_pos = 0;
    while (pos < length) {
        skip();
        if (pos >= length) break;
        if (text[pos] == '#') {                       // comment: skip to the newline
            while (pos < length && text[pos] != '\n') pos++;
            continue;
        }
        if (text[pos] != '\n') {                      // (a blank line just falls through)
            Number v = expr();
            if (error_pos) return;
            skip();
            if (pos < length && text[pos] != '\n') { fail(pos + 1); return; }  // e.g. extra ')'
            if (v.is_float) fprintf(out, "%.15g\n", v.d);
            else fprintf(out, "%lld\n", v.i);
        }
        if (pos < length) pos++;                      // skip the '\n'
    }
}

// Last part of a path without trailing '/', and without ".txt" if cut_txt.
static void base_name(const char *path, char *base, int cut_txt) {
    int n = (int)strlen(path);
    while (n > 1 && path[n - 1] == '/') n--;
    int start = n;
    while (start > 0 && path[start - 1] != '/') start--;
    snprintf(base, 1024, "%.*s", n - start, path + start);
    n = (int)strlen(base);
    if (cut_txt && n >= 4 && strcmp(base + n - 4, ".txt") == 0) base[n - 4] = '\0';
}

// Read the whole file with ONE fread (no copying line by line).
static int read_file(const char *path) {
    struct stat st;
    FILE *f = fopen(path, "rb");
    if (!f || (stat(path, &st) == 0 && S_ISDIR(st.st_mode))) {
        fprintf(stderr, "calc: cannot read '%s'\n", path);
        if (f) fclose(f);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    rewind(f);
    text = malloc((size_t)length + 1);
    if (!text || fread(text, 1, (size_t)length, f) != (size_t)length) {
        fprintf(stderr, "calc: cannot read '%s'\n", path);
        fclose(f);
        free(text);
        return -1;
    }
    fclose(f);
    text[length] = '\0';
    return 0;
}

// Evaluate one file into <out_dir>/<base>_<name>_<lastname>_<id>.txt
static int process_file(const char *path, const char *out_dir) {
    struct stat st;
    if (read_file(path) != 0) return -1;
    if (stat(out_dir, &st) != 0 && mkdir(out_dir, 0777) != 0) {
        fprintf(stderr, "calc: cannot create folder '%s'\n", out_dir);
        free(text);
        return -1;
    }
    char base[1024], out_path[4096];
    base_name(path, base, 1);
    snprintf(out_path, sizeof out_path, "%s/%s_%s_%s_%s.txt", out_dir, base, NAME, LAST, ID);
    FILE *out = fopen(out_path, "w");
    if (!out) {
        fprintf(stderr, "calc: cannot write '%s'\n", out_path);
        free(text);
        return -1;
    }
    run(out);
    if (error_pos) {     // on an error the file holds ONLY the error: start it over
        fclose(out);
        out = fopen(out_path, "w");
        if (!out) { free(text); return -1; }
        fprintf(out, "ERROR:%ld\n", error_pos);
    }
    free(text);
    return fclose(out) == 0 ? 0 : -1;
}

int main(int argc, char **argv) {
    const char *dir = NULL, *out_dir = NULL, *input = NULL;
    for (int i = 1; i < argc; i++) {
        int is_d = !strcmp(argv[i], "-d") || !strcmp(argv[i], "--dir");
        int is_o = !strcmp(argv[i], "-o") || !strcmp(argv[i], "--output-dir");
        if (is_d || is_o) {
            if (++i >= argc) { fprintf(stderr, "calc: missing value after '%s'\n", argv[i - 1]); return 2; }
            if (is_d) dir = argv[i]; else out_dir = argv[i];
        } else if (argv[i][0] == '-' || input) {
            fprintf(stderr, "calc: unknown option or extra argument '%s'\n", argv[i]);
            return 2;
        } else {
            input = argv[i];
        }
    }
    if ((dir == NULL) == (input == NULL)) {      // need exactly one of them
        fprintf(stderr, "usage: calc [-d DIR] [-o OUTDIR] [input.txt]\n");
        return 2;
    }

    char default_out[2048];
    if (!out_dir) {                              // default: <input_base>_<name>_<id>
        char base[1024];
        base_name(dir ? dir : input, base, dir == NULL);
        snprintf(default_out, sizeof default_out, "%s_%s_%s", base, NAME, ID);
        out_dir = default_out;
    }
    if (input) return process_file(input, out_dir) == 0 ? 0 : 1;

    DIR *d = opendir(dir);
    if (!d) { fprintf(stderr, "calc: cannot open folder '%s'\n", dir); return 1; }
    int rc = 0;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        size_t n = strlen(e->d_name);
        char path[4096];
        struct stat st;
        if (n <= 4 || strcmp(e->d_name + n - 4, ".txt") != 0) continue;
        snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) continue;   // skip subfolders
        if (process_file(path, out_dir) != 0) rc = 1;
    }
    closedir(d);
    return rc;
}