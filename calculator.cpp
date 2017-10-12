#include <cmath>
#include <deque>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <stack>
#include <string>

typedef double (*EvaluateFunc)(const double &a, const double &b);

struct OpContract {
	EvaluateFunc func;
	int precedence;
	bool leftAssociativity;
	OpContract() {}
	OpContract(EvaluateFunc f, int p, bool leftAssoc)
	    : func(f), precedence(p), leftAssociativity(leftAssoc) {}
};

static std::map<char, OpContract> opContracts = {
    {'+', OpContract([](const double &a, const double &b) { return a + b; }, 1,
                     true)},
    {'-', OpContract([](const double &a, const double &b) { return a - b; }, 1,
                     true)},
    {'*', OpContract([](const double &a, const double &b) { return a * b; }, 2,
                     true)},
    {'/', OpContract([](const double &a, const double &b) { return a / b; }, 2,
                     true)},
    {'^', OpContract([](const double &a, const double &b) { return pow(a, b); },
                     3, false)}};

enum TokenType { LEFT_BRACKET, RIGHT_BRACKET, NUMBER, OPERATOR };

struct Token {
	TokenType type;
	char symbol;
	double value;
};

std::deque<Token> tokenise(std::string input) {
	std::istringstream stream(input);
	std::deque<Token> tokens;
	while (stream.good()) {
		char peek = stream.peek();
		if (stream.eof()) {
			// Reached end
			break;
		}
		if (peek == ' ') {
			stream.ignore(1);
			continue;
		}
		Token t;
		if (peek == '(') {
			// Left bracket
			t.type = LEFT_BRACKET;
			stream.ignore(1);
		} else if (peek == ')') {
			// Right bracket
			t.type = RIGHT_BRACKET;
			stream.ignore(1);
		} else if ((peek == '+' || peek == '-') &&
		           (tokens.empty() || (tokens.back().type != NUMBER))) {
			// Positive or negative number
			t.type = NUMBER;
			stream >> t.value;
			if (stream.fail()) {
				std::cerr << "Invalid token" << std::endl;
				exit(EXIT_FAILURE);
			}
		} else if (isdigit(peek)) {
			// Number
			t.type = NUMBER;
			stream >> t.value;
		} else {
			// Operator
			t.type = OPERATOR;
			t.symbol = peek;
			stream.ignore(1);
		}

		// Push token to
		tokens.push_back(t);
	}
	return tokens;
}

double evaluate(double left, char op, double right) {
	// Find contract
	std::map<char, OpContract>::iterator search = opContracts.find(op);
	if (search != opContracts.end()) {
		// Evaluate with function pointer
		return search->second.func(left, right);
	} else {
		std::cerr << "Operator " << op << " is not supported" << std::endl;
		exit(EXIT_FAILURE);
	}
}

double expression(std::deque<Token> &tokens) {
	// Dijkstra's Shunting-yard algorithm
	std::queue<double> output;
	std::stack<Token> operators;

	for (const Token &token : tokens) {
		if (token.type == NUMBER) {
			// Push number to number stream
			output.push(token.value);
		} else if (token.type == OPERATOR) {
			// While there is on operator on the stack with higher precendence
			while (!operators.empty()) {
				OpContract contract1 = opContracts[token.symbol];
				Token op2 = operators.top();
				if (op2.type != OPERATOR) {
					break;
				}
				OpContract contract2 = opContracts[op2.symbol];
				if ((contract1.leftAssociativity &&
				     contract1.precedence <= contract2.precedence) ||
				    (!contract1.leftAssociativity &&
				     contract1.precedence < contract2.precedence)) {
					operators.pop();

					// Get operands from stack and evaluate
					double left = output.front();
					output.pop();
					double right = output.front();
					output.pop();

					output.push(evaluate(left, op2.symbol, right));
				} else {
					// No operators on stack with higher precendence
					break;
				}
			}
			// Push operator to operator stack
			operators.push(token);
		} else if (token.type == LEFT_BRACKET) {
			// Push opening bracket to stack
			operators.push(token);
		} else if (token.type == RIGHT_BRACKET) {
			// Evaluate until we reach a left bracket
			while (!operators.empty() && operators.top().type != LEFT_BRACKET) {
				// Get operator from operator stack
				Token op = operators.top();
				operators.pop();

				// Get operands from output stream
				double left = output.front();
				output.pop();
				double right = output.front();
				output.pop();

				// Evaluate
				output.push(evaluate(left, op.symbol, right));
			}
			if (operators.empty()) {
				// Didn't find a left bracket, mismatch
				std::cerr << "Mismatched parenthesis" << std::endl;
				exit(EXIT_FAILURE);
			}
			// Drop left bracket
			operators.pop();
		}
	}

	while (!operators.empty()) {
		Token token = operators.top();
		operators.pop();

		if (token.type == LEFT_BRACKET || token.type == RIGHT_BRACKET) {
			// Mismatched parenthesis
			std::cerr << "Mismatched parenthesis" << std::endl;
			exit(EXIT_FAILURE);
		}

		if (output.size() < 2) {
			std::cerr << "Mismatched operands" << std::endl;
			exit(EXIT_FAILURE);
		}

		double left = output.front();
		output.pop();
		double right = output.front();
		output.pop();

		output.push(evaluate(left, token.symbol, right));
	}

	return output.front();
}

int main() {
	std::string input;
	while (true) {
		std::getline(std::cin, input);

		if (std::cin.eof()) {
			return EXIT_SUCCESS;
		}

		if (input.empty()) {
			continue;
		}

		std::deque<Token> tokens = tokenise(input);
		std::cout << expression(tokens) << std::endl;
	}

	return EXIT_SUCCESS;
}
