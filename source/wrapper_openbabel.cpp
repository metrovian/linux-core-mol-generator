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

extern char *mol_create(const char *name, const char *inchi) {
	OpenBabel::OBMol mol;
	OpenBabel::OBConversion inconv;
	OpenBabel::OBFormat *input = inconv.FindFormat("inchi");
	if (!input) {
		log_error("failed to find inchi format");
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
	if (!inconv.ReadString(&mol, inchi)) {
		log_error("failed to parse inchi format");
		return NULL;
	}

	if (!mol.AddHydrogens()) {
		log_error("failed to add hydrogens");
		return NULL;
	}

	OpenBabel::OBBuilder builder;
	if (!builder.Build(mol)) {
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

extern char *mol_hash(const char *inchi) {
	OpenBabel::OBMol mol;
	OpenBabel::OBConversion inconv;
	OpenBabel::OBFormat *input = inconv.FindFormat("inchi");
	if (!input) {
		log_error("failed to find inchi format");
		return NULL;
	}

	OpenBabel::OBConversion outconv;
	OpenBabel::OBFormat *output = outconv.FindFormat("inchikey");
	if (!output) {
		log_error("failed to find inchikey format");
		return NULL;
	}

	inconv.SetInFormat(input);
	outconv.SetOutFormat(output);
	if (!inconv.ReadString(&mol, inchi)) {
		log_error("failed to parse inchi format");
		return NULL;
	}

	std::string keystr = outconv.WriteString(&mol);
	keystr.erase(keystr.find_last_not_of(" \n\r\t") + 1);
	return strdup(keystr.c_str());
}