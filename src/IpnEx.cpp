#include "IpnEx.h"

void IpnEx::HandleError() const
{
	printf("Error in line %d: ", LineNum);
	PrintError();
}

void IpnExNotInt::PrintError() const
{
	if (where == InFstDim) {
		printf("integer value expected ");
		printf("as first dimension of array ");
	} else if (where == InScdDim) {
		printf("integer value expected ");
		printf("as second dimension of array ");
	} else if (where == InVarDesc) {
		printf("integer value expected ");
		printf("as initial value in description of integer ");
	} else if (where == InAssign) {
		printf("expression after \"=\" is not integer type ");
	}
	printf("\n");
}

void IpnExNotIntOrReal::PrintError() const
{
	if (where == InAssign) {
		printf("expression after \"=\" is not integer or real type ");
	} else if (where == InVarDesc) {
		printf("integer or real value expected ");
		printf("as initial value in description of real ");
	} else if (where == InAddFst) {
		printf("first operand in addition is not integer or real ");
	} else if (where == InAddScd) {
		printf("second operand in addition is not integer or real ");
	} else if (where == InSubFst) {
		printf("first operand in subtraction is not integer or real ");
	} else if (where == InSubScd) {
		printf("second operand in subtraction is not integer or real ");
	} else if (where == InMulFst) {
		printf("first operand in multiplication is not integer or real ");
	} else if (where == InMulScd) {
		printf("second operand in multiplication is not integer or real ");
	} else if (where == InDivFst) {
		printf("first operand in division is not integer or real ");
	} else if (where == InDivScd) {
		printf("second operand in division is not integer or real ");
	} else if (where == InOrFst) {
		printf("first operand in \"or\" is not integer or real ");
	} else if (where == InOrScd) {
		printf("second operand in \"or\" is not integer or real ");
	} else if (where == InAndFst) {
		printf("first operand in \"and\" is not integer or real ");
	} else if (where == InAndScd) {
		printf("second operand in \"and\" is not integer or real ");
	} else if (where == InLessFst) {
		printf("first operand in \"<\" is not integer or real ");
	} else if (where == InLessScd) {
		printf("second operand in \"<\" is not integer or real ");
	} else if (where == InLessEFst) {
		printf("first operand in \"<=\" is not integer or real ");
	} else if (where == InLessEScd) {
		printf("second operand in \"<=\" is not integer or real ");
	} else if (where == InGreatFst) {
		printf("first operand in \">\" is not integer or real ");
	} else if (where == InGreatScd) {
		printf("second operand in \">\" is not integer or real ");
	} else if (where == InGreatEFst) {
		printf("first operand in \">=\" is not integer or real ");
	} else if (where == InGreatEScd) {
		printf("second operand in \">=\" is not integer or real ");
	} else if (where == InEqualFst) {
		printf("first operand in \"==\" is not integer or real ");
	} else if (where == InEqualScd) {
		printf("second operand in \"==\" is not integer or real ");
	} else if (where == InNotEqualFst) {
		printf("first operand in \"=!\" is not integer or real ");
	} else if (where == InNotEqualScd) {
		printf("second operand in \"=!\" is not integer or real ");
	} else if (where == InNeg) {
		printf("operand in \"!\" is not integer or real ");
	} else if (where == InUnSub) {
		printf("operand in \"-\" is not integer or real ");
	} else if (where == InOpGoFalse) {
		printf("operand in conditional jump is not integer or real ");
	}
	printf("\n");
}

void IpnExNotStr::PrintError() const
{
	if (where == InAssign) {
		printf("expression after \"=\" is not string type ");
	} else if (where == InVarDesc) {
		printf("string value expected ");
		printf("as initial value in description of string ");
	}
	printf("\n");
}

void IpnExNotIntOrRealOrString::PrintError() const
{
	if (where == InPut) {
		printf("integer or real or string value expected ");
		printf("as argument of put ");
	} else if (where == InSemicolon) {
		printf("integer or real or string value expected before \";\" ");
	}
	printf("\n");
}

void IpnExNotFound::PrintError() const
{
	if (where == InVar || where == InAssign) {
		printf("variable \"%s\" was not declared in this scope ", name);
	}
	printf("\n");
}

void IpnExRedec::PrintError() const
{
	if (where == InVarDesc) {
		printf("variable \"%s\" is already declared ", name);
	}
	printf("\n");
}

void IpnExSegFault::PrintError() const
{
	if (name) {
		printf("%s: Segmentation fault\n", name);
	} else {
		printf("Segmentation fault\n");
	}
}

void IpnExNotLabel::PrintError() const
{
	if (where == InOpGo) {
		printf("operand in unconditional jump is not label ");
	} else if (where == InOpGoFalse) {
		printf("operand in conditional jump is not label ");
	}
	printf("\n");
}

void IpnExNotVarAddr::PrintError() const
{
	if (where == InAssign) {
		printf("operand in assignment is not address of variable ");
	} else if (where == InTakeValue) {
		printf("operand in taking of value is not address of variable ");
	}
	printf("\n");
}
