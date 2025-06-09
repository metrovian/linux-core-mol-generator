CREATE EXTENSION IF NOT EXISTS vector;

CREATE TABLE molecule (
    id SERIAL PRIMARY KEY,
    name TEXT,
    smiles TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(name, smiles)
);

CREATE TABLE mass_spectrum (
    id SERIAL PRIMARY KEY,
    molecule_id INTEGER REFERENCES molecule(id),
    spectrum_vector vector(1000),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);