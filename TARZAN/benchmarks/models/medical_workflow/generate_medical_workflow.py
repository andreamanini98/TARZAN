#!/usr/bin/env python3
"""
Generate XTA + query (.q) files for a hospital-like timed automata model.

For N = 2, 4, ..., 30, this script generates:
  - hospital_N.xta
  - hospital_N.q
in the 'generated_models' directory.
"""

from pathlib import Path

OUTPUT_DIR = Path("generated_models")
OUTPUT_DIR.mkdir(exist_ok=True)

HEADER = """// Place global declarations here.
chan checkID, medicalCheck,
"""

CONSTANTS = """// Time variables
const int patArivMin = 9;
const int patArivMax = 9;
const int readDocsMin = 1;
const int checkIDMin = 2;
const int docMaxDeadline = 25;
const int nurseMaxDeadline = 5;
const int patientDeadline = 9;

int docReady;
int nurseReady;
"""

PATIENT_TEMPLATE = """process Patient{idx}() {{

clock x, patientFlow;
state
    Done,
    WaitingMedicalCheck {{ x <= patientDeadline }},
    Waiting {{ x <= patientDeadline }},
    Ready {{ patientFlow <= patArivMax }},
    L0;
urgent
    L0;
init
    Ready;
trans
    L0 -> Waiting {{ sync patientArrivalNurse{idx}!; }},
    Ready -> Ready {{ guard patientFlow >= patArivMax; assign patientFlow = 0; }},
    WaitingMedicalCheck -> Done {{ sync medicalCheck?; }},
    Waiting -> WaitingMedicalCheck {{ sync checkID?; assign x=0; }},
    Ready -> L0 {{ guard patientFlow >= patArivMin && docReady == 1 && nurseReady == 1; sync patientArrivalDoc{idx}!; assign x = 0, patientFlow = 0; }};
}}
"""

DOCTOR_TEMPLATE = """process Doctor() {{

clock x;
state
    Prepaired {{ x <= docMaxDeadline }},
    GoToPatient,
    Idle,
    L0;
urgent
    L0;
init
    L0;
trans
    L0 -> Idle {{ assign docReady = 1; }},
    Prepaired -> Idle {{ sync medicalCheck!; assign docReady = 1; }},
    GoToPatient -> Prepaired {{ guard x >= readDocsMin; assign x = 0; }},
{transitions};
}}
"""

NURSE_TEMPLATE = """process Nurse() {{

clock x;
state
    atPatient {{ x <= nurseMaxDeadline }},
    GoToPatient,
    Idle,
    L0;
urgent
    L0;
init
    L0;
trans
    L0 -> Idle {{ assign nurseReady = 1; }},
    atPatient -> Idle {{ assign nurseReady = 1; }},
    GoToPatient -> atPatient {{ guard x >= checkIDMin; sync checkID!; assign x = 0; }},
{transitions};
}}
"""

def generate_xta(n: int) -> str:
    """Generate the full XTA source for n patients."""
    # --- Global channels ---
    chan_lines = [f"     patientArrivalDoc{i:03d}, patientArrivalNurse{i:03d}," for i in range(1, n + 1)]
    chan_block = HEADER + "\n".join(chan_lines)[:-1] + ";\n\n" + CONSTANTS

    # --- Patients ---
    patients_block = "\n".join(PATIENT_TEMPLATE.format(idx=f"{i:03d}") for i in range(1, n + 1))

    # --- Doctor transitions ---
    doc_trans = ",\n".join(
        f"    Idle -> GoToPatient {{ sync patientArrivalDoc{i:03d}?; assign x = 0, docReady = 0; }}"
        for i in range(1, n + 1)
    )
    doctor_block = DOCTOR_TEMPLATE.format(transitions=doc_trans)

    # --- Nurse transitions ---
    nurse_trans = ",\n".join(
        f"    Idle -> GoToPatient {{ sync patientArrivalNurse{i:03d}?; assign x = 0, nurseReady = 0; }}"
        for i in range(1, n + 1)
    )
    nurse_block = NURSE_TEMPLATE.format(transitions=nurse_trans)

    # --- System composition ---
    system_block = "system " + ", ".join(
        [f"Patient{i:03d}" for i in range(1, n + 1)] + ["Nurse", "Doctor"]
    ) + ";"

    return "\n".join([chan_block, patients_block, nurse_block, doctor_block, system_block])


def generate_query(n: int) -> str:
    """Generate a reachability query checking that all patients reach Done."""
    conjuncts = " and ".join([f"Patient{i:03d}.Done" for i in range(1, n + 1)])
    return f"E<> ( {conjuncts} )\n"


def main():
    for n in range(10, 101, 10):
        xta_code = generate_xta(n)
        q_code = generate_query(n)

        xta_file = OUTPUT_DIR / f"medical_workflow_{n:03d}.xta"
        q_file = OUTPUT_DIR / f"medical_workflow_{n}.q"

        xta_file.write_text(xta_code)
        q_file.write_text(q_code)

        print(f"✅ Generated {xta_file.name} and {q_file.name}")

if __name__ == "__main__":
    main()