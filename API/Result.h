#include "../Interpreter/llvmsql.h"
#include "../RecordManager/datatype.h"

using namespace std;
enum result_token {
	res_INT = 0,
	res_FLOAT = 1,
	res_STRING = 2,
	res_TABLE_COL = 3,
	res_ID = 4,
	res_TABLE = 5,
	res_COLUMN = 6,
	res_PREDICATE = 7,
	res_SELECT = 8,
};
class Result
{
public:
	Result();

	Result(int int_val);
	Result(float float_val);
	Result(string string_val);
	
	Result(std::shared_ptr<TablecolAST> tablecol , TablecolAST t);
	Result(std::shared_ptr<IdAST> id , IdAST i);
	Result(std::shared_ptr<PredicateAST> predicate , PredicateAST p);
	Result(std::vector<map<string, BaseData*>> res);

	int type;

	int int_val;
	float float_val;
	string string_val;

	vector<map<string, BaseData*>> result;


	std::shared_ptr<TablecolAST> tablecol;
	std::shared_ptr<IdAST> id;
	std::shared_ptr<PredicateAST> predicate;



};

