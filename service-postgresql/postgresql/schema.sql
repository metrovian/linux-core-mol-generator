CREATE EXTENSION IF NOT EXISTS vector;

CREATE TABLE molecule (
    id SERIAL PRIMARY KEY,
    name TEXT,
    smiles TEXT
);

CREATE TABLE mass_spectrum (
    id SERIAL PRIMARY KEY,
    compound_id INTEGER REFERENCES molecule(id),
    description TEXT,
    spectrum_vector vector(1000),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);