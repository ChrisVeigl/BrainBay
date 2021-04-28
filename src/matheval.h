/* -----------------------------------------------------------------------------

  matheval.h
	header file for the GNU LibMatheval- Library for Expression Evaluation
			   
  Authors: Aleksandar B. Samardz 
  Link to GNU-Libmatheval: http://www.gnu.org/software/libmatheval
 

Description:
  String representation of function is allowed to consist of decimal constants, 
  variables, elementary functions, unary and binary operations. 

  Variable name is any combination of alphanumericals and _ characters beginning 
  with a non-digit that is not elementary function name. 

Supported elementary functions are 
(names that should be used are given in parenthesis): 
  exponential (exp), logarithmic (log), square root (sqrt), sine (sin), cosine (cos), 
  tangent (tan), cotangent (cot), secant (sec), cosecant (csc), 
  inverse sine (asin), inverse cosine (acos), inverse tangent (atan), 
  inverse cotangent (acot), inverse secant (asec), inverse cosecant (acsc), 
  hyperbolic sine (sinh), cosine (cosh), hyperbolic tangent (tanh), 
  hyperbolic cotangent (coth), hyperbolic secant (sech), hyperbolic cosecant (csch), 
  hyperbolic inverse sine (asinh), hyperbolic inverse cosine (acosh), 
  hyperbolic inverse tangent (atanh), hyperbolic inverse cotangent (acoth), 
  hyperbolic inverse secant (asech), hyperbolic inverse cosecant (acsch), 
  absolute value (abs), Heaviside step function (step) with value 1 defined 

  for x = 0 and Dirac delta function with infinity (delta) and 
  not-a-number (nandelta) values defined for x = 0. 

Supported unary operation is unary minus ('-'). 

Supported binary operations are:
  addition ('+'), subtraction ('+'), multiplication ('*'), 
  division multiplication ('/') and exponentiation ('^'). 

Usual mathematical rules regarding operation precedence apply. 
Parenthesis ('(' and ')') could be used to change priority order. 

Blanks and tab characters are allowed in string representing function; 
newline characters must not appear in this string. 


This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; See the
GNU General Public License for more details.

  
-------------------------------------------------------------------------------------*/


#ifndef EVALUATOR_H
#define EVALUATOR_H 1

#ifdef __cplusplus
extern          "C" {
#endif

	/*
	 * Create evaluator from string representing function.  Function
	 * returns pointer that should be passed as first argument to all
	 * other library functions.  If an error occurs, function will return
	 * null pointer.
	 */
	extern void    *evaluator_create(char *string);

	/*
	 * Destroy evaluator specified.
	 */
	extern void     evaluator_destroy(void *evaluator);

	/*
	 * Evaluate function represented by evaluator given.  Variable names
	 * and respective values are represented by function third and fourth
	 * argument. Number of variables i.e. length of these two arrays is
	 * given by second argument.  Function returns evaluated function
	 * value.  In case that function contains variables with names not
	 * given through third function argument, value of this variable is
	 * undeterminated.
	 */
	extern double   evaluator_evaluate(void *evaluator, int count, char **names, double *values);

	/*
	 * Calculate length of textual representation of evaluator. This
	 * procedure is intended to be used along with evaluator_write()
	 * procedure that follows.
	 */
	extern int      evaluator_calculate_length(void *evaluator);

	/*
	 * Write textual representation of evaluator (i.e. corresponding
	 * function) to given string.  No string overflow is checked by this
	 * procedure; string of appropriate length (calculated beforehand
	 * using above evaluator_calculate_length() procedure) is expected to
	 * be allocated beforehand.
	 */
	extern void     evaluator_write(void *evaluator, char *length);

	/*
	 * Create evaluator for first derivative of function represented by
	 * evaluator given as first argument using derivative variable given
	 * as second argument.
	 */
	extern void    *evaluator_derivative(void *evaluator, char *name);

	/*
	 * Helper functions to simplify evaluation when variable names are
	 * "x", "x" and "y" or "x" and "y" and "z" respectively.
	 */
	extern double   evaluator_evaluate_x(void *evaluator, double x);
	extern double   evaluator_evaluate_x_y(void *evaluator, double x, double y);
	extern double   evaluator_evaluate_x_y_z(void *evaluator, double x, double y, double z);

	/*
	 * Helper functions to simplify differentiation when variable names
	 * are "x" or "y" or "z" respectively.
	 */
	extern void    *evaluator_derivative_x(void *evaluator);
	extern void    *evaluator_derivative_y(void *evaluator);
	extern void    *evaluator_derivative_z(void *evaluator);

#ifdef __cplusplus
}
#endif
#endif
