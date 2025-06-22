const CONFIG_SPECTRUM = {
	mass: {
		placeholderX: "(0-1000)",
		placeholderY: "(0-1000)",
		vectorSize: 1000,
		yFixed: false,
		validate: (x, y) => x >= 0 && x < 1000 && y >= 0 && y <= 999
	},
	nmr: {
		placeholderX: "(0-1500)",
		placeholderY: "(1)",
		vectorSize: 1500,
		yFixed: true,
		validate: (x, y) => x >= 0 && x < 1500
	},
	optics: {
		placeholderX: "(0-2000)",
		placeholderY: "(0-1000)",
		vectorSize: 2000,
		yFixed: false,
		validate: (x, y) => x >= 0 && x < 2000 && y >= 0 && y <= 999
	}
};

let peaks = [];

function getSpecType() {
	return document.querySelector('input[name="specType"]:checked').value;
}

function getSpecConfig() {
	return CONFIG_SPECTRUM[getSpecType()];
}

function updatePlaceholders() {
	const { placeholderX, placeholderY, yFixed } = getSpecConfig();
	const xInput = document.getElementById('xInput');
	const yInput = document.getElementById('yInput');
	xInput.placeholder = placeholderX;
	yInput.placeholder = placeholderY;
	yInput.disabled = yFixed;
}

function addPeak() {
	const { yFixed, vectorSize, validate } = getSpecConfig();
	const x = parseInt(document.getElementById('xInput').value);
	let y = yFixed ? 1 : parseInt(document.getElementById('yInput').value);
	if (!validate(x, y)) {
		alert("Invalid values. Please check input range.");
		return;
	}

	peaks.push({ x, y });
	sortPeaks();
	updateTextArea();
	document.getElementById('xInput').value = '';
	document.getElementById('yInput').value = '';
}

function sortPeaks() {
	peaks.sort((a, b) => a.x - b.x);
}

function updateTextArea() {
	const textarea = document.getElementById('peakTextArea');
	const lines = peaks.map(p => `${Math.floor(p.x + 0.5)} ${p.y}`);
	textarea.value = lines.join('\n');
}

function updateFromText() {
	const { yFixed, vectorSize, validate } = getSpecConfig();
	const textarea = document.getElementById('peakTextArea');
	const lines = textarea.value.trim().split('\n');
	const newPeaks = [];
	for (let i = 0; i < lines.length; i++) {
		const line = lines[i].trim();
		if (line === '') {
                        continue;
                }

		const parts = line.split(' ');
		if (parts.length < 1 || parts.length > 2) {
			alert(`Line ${i + 1} is invalid: "${line}"`);
			return;
		}

		const x = Math.floor(parseFloat(parts[0]) + 0.5);
		const y = yFixed ? 1 : parseInt(parts[1]);
		if (!validate(x, y)) {
			alert(`Line ${i + 1} has invalid values: "${line}"`);
			return;
		}
                
		newPeaks.push({ x, y });
	}

	peaks = newPeaks;
	sortPeaks();
	updateTextArea();
}

function submitData() {
	updateFromText();
	if (peaks.length === 0) {
		alert("Please add at least one peak before generating.");
		return;
	}

	const selectedType = getSpecType();
	const { vectorSize } = getSpecConfig();
	const data = Array(vectorSize).fill(0);
	peaks.forEach(({ x, y }) => {
		if (x < vectorSize) {
			data[x] = y;
		}
	});

	const rawText = data.join(",");
	fetch(`/api/${selectedType}`, {
		method: 'POST',
		headers: { 'Content-Type': 'text/plain' },
		body: rawText
	}).then(response => {
		if (!response.ok) throw new Error("API server error");
		return response.text();
	}).then(text => {
		const blob = new Blob([text], { type: 'chemical/x-mdl-molfile' });
		const link = document.createElement('a');
		link.href = URL.createObjectURL(blob);
		link.download = 'generated.mol';
		link.click();
		URL.revokeObjectURL(link.href);
	}).catch(err => {
		alert("Generate failed: " + err.message);
	});
}

document.querySelectorAll('input[name="specType"]').forEach(radio => {
	radio.addEventListener('change', updatePlaceholders);
});

document.addEventListener('DOMContentLoaded', updatePlaceholders);