#include "Result.h"

using namespace std;

Result::Result()
{
	Result::type = tok_INT;
	Result::int_val = 0;
}

Result::Result(int int_val)
{
	Result::type = tok_INT;
}

Result::Result(float float_val)
{
	Result::type = tok_FLOAT;
}

Result::Result(string string_val)
{
	Result::type = res_STRING;
}

Result::Result(std::shared_ptr<TablecolAST> tablecol , TablecolAST t)
{
	Result::type = res_TABLE_COL;
	Result::tablecol = tablecol;
}

Result::Result(std::shared_ptr<IdAST> id ,IdAST i) {
	Result::type = res_ID;
	Result::id = id;
}

Result::Result(std::shared_ptr<PredicateAST> predicate, PredicateAST p)
{
	Result::type = res_PREDICATE;
	Result::predicate = predicate;
}

Result::Result(std::vector<map<string, BaseData*>> res)
{
	Result::result = res;
}
