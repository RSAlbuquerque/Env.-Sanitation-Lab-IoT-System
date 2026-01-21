import pandas as pd
from pathlib import Path
from openpyxl import load_workbook
import tkinter as tk
from tkinter import messagebox

# Folder containing CSV files (same folder as the script)
base_path = Path(__file__).parent
input_path = base_path / "csv"
output_path = base_path / "excel"

input_path.mkdir(exist_ok=True)
output_path.mkdir(exist_ok=True)

# Mapping old headers to new headers
hydro_header_mapping = {
    "created_at": "Date",
    "entry_id": "Sample Num.",
    "field1": "Temperature ºC",
    "field2": "TDS (uS/cm)",
    "field3": "pH",
    "field4": "Conductivity (uS/cm)",
    "field5": "DO (mg/L)",
    "field6": "ORP (mV)",
    "field7": "Chlorine (mg/L)",
}

air_header_mapping = {
    "created_at": "Date",
    "entry_id": "Sample Num.",
    "field1": "PM2.5 (ug/m3)",
    "field2": "PM10 (ug/m3)",
    "field3": "Toluene (ppm)",
    "field4": "CO (ppm)",
    "field5": "CO2 (ppm)",
    "field6": "Alcohol (ppm)",
    "field7": "NH4 (ppm)",
    "field8": "Acetone (ppm)",
}

# Variables used to output the summary
results = []
hydro_count = 0
air_count = 0
failed_count = 0

# Loop through all CSV files in the folder
for csv_file in input_path.glob("*.csv"):
    try:
        df = pd.read_csv(csv_file)

        num_columns = len(df.columns)
        if num_columns == 9:
            df = df.rename(columns=hydro_header_mapping)
            csv_type = "hydro"
            hydro_count += 1
        elif num_columns == 10:
            df = df.rename(columns=air_header_mapping)
            csv_type = "air"
            air_count += 1
        else:
            results.append(f"Skipped {csv_file.name} - unknown format ({num_columns} columns)")
            failed_count += 1
            continue

        excel_file = output_path / csv_file.with_suffix(".xlsx").name
        df.to_excel(excel_file, index=False)

        wb = load_workbook(excel_file)
        ws = wb.active

        for col in ws.columns:
            max_length = 22
            column_letter = col[0].column_letter
            ws.column_dimensions[column_letter].width = max_length

        wb.save(excel_file)
        results.append(f"{csv_file.name} to {excel_file.name} ({csv_type})")
    except Exception as e:
        results.append(f"Failed: {csv_file.name} - {e}")
        failed_count += 1

root = tk.Tk()
root.withdraw()

summary = f"Conversion Complete!\n\n"
summary += f"Hydro files: {hydro_count}\n"
summary += f"Air files: {air_count}\n"
summary += f"Failed/Skipped: {failed_count}\n\n"
summary += "Details:\n" + "\n".join(results)

messagebox.showinfo("CSV to Excel Converter", summary)
