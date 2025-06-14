#include "wrapper_openbabel.h"
#include "wrapper_spdlog.h"
#include "predefined.h"
#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#include <openbabel/op.h>
#include <openbabel/forcefield.h>
#include <openbabel/builder.h>
#include <cstdlib>
#include <cstring>

extern char *mol_create(const char *name, const char *smiles) {
	OpenBabel::OBMol mol;
	OpenBabel::OBConversion inconv;
	OpenBabel::OBFormat *input = inconv.FindFormat("smi");
	if (!input) {
		log_error("failed to find smiles format");
		return NULL;
	}

	OpenBabel::OBConversion outconv;
	OpenBabel::OBFormat *output = outconv.FindFormat("mol");
	if (!output) {
		log_error("failed to find mol format");
		return NULL;
	}

	inconv.SetInFormat(input);
	outconv.SetOutFormat(output);
	if (!inconv.ReadString(&mol, smiles)) {
		log_error("failed to parse smiles format");
		return NULL;
	}

	if (!mol.AddHydrogens()) {
		log_error("failed to add hydrogens");
		return NULL;
	}

	OpenBabel::OBBuilder builder;
	if (!builder.Build(mol, false)) {
		log_error("failed to build molecular structure");
		return NULL;
	}

	OpenBabel::OBForceField *field = OpenBabel::OBForceField::FindForceField("mmff94");
	if (!field->Setup(mol)) {
		log_error("failed to optimize molecular structure");
		return NULL;
	}

	field->ConjugateGradients(500);
	field->GetCoordinates(mol);
	mol.SetTitle(name);
	std::string molstr = outconv.WriteString(&mol, true);
	return strdup(molstr.c_str());
}