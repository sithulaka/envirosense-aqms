import pandas as pd
import tkinter as tk
from tkinter import ttk, messagebox, filedialog
from datetime import datetime
from openpyxl.styles import numbers

class LiupeConverter:
    def __init__(self, root):
        self.root = root
        self.setup_ui()
        
    def setup_ui(self):
        # Window configuration
        self.root.title("Liupe Tech - Data Converter")
        self.root.geometry("500x280")
        self.root.resizable(False, False)
        self.root.configure(bg='#f5f5f5')
        
        # Custom colors
        self.color_primary = '#2c3e50'
        self.color_secondary = '#3498db'
        self.color_light = '#ecf0f1'
        
        # Main container
        main_frame = tk.Frame(self.root, bg=self.color_light)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)
        
        # Centered header
        header_frame = tk.Frame(main_frame, bg=self.color_light)
        header_frame.pack(fill=tk.X, pady=(0, 15))
        
        # Centered business name
        tk.Label(
            header_frame,
            text="Liupe Technology",
            fg=self.color_primary,
            bg=self.color_light,
            font=('Segoe UI', 14, 'bold'),
        ).pack(expand=True)
        
        # Divider line
        ttk.Separator(
            main_frame,
            orient='horizontal'
        ).pack(fill=tk.X, pady=5)
        
        # Content
        tk.Label(
            main_frame,
            text="Sensor Data Converter",
            fg=self.color_primary,
            bg=self.color_light,
            font=('Segoe UI', 10, 'bold')
        ).pack(pady=(5, 0))
        
        tk.Label(
            main_frame,
            text="Convert TXT to Excel with proper date/time formatting",
            fg='#555555',
            bg=self.color_light,
            font=('Segoe UI', 9)
        ).pack(pady=(0, 20))
        
        # Convert button
        convert_btn = tk.Button(
            main_frame,
            text="CONVERT DATA",
            command=self.convert_data,
            bg=self.color_secondary,
            fg='white',
            activebackground='#2980b9',
            borderwidth=0,
            font=('Segoe UI', 10, 'bold'),
            padx=25,
            pady=8
        )
        convert_btn.pack(pady=5)
        
        # Status bar
        self.status = tk.Label(
            main_frame,
            text="Ready to convert",
            fg='#555555',
            bg=self.color_light,
            font=('Segoe UI', 8),
            anchor=tk.W
        )
        self.status.pack(fill=tk.X, pady=(15, 0))
    
    def convert_data(self):
        try:
            self.status.config(text="Selecting input file...", fg='#555555')
            self.root.update()
            
            input_file = filedialog.askopenfilename(
                title="Select sensor data file",
                filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
            )
            if not input_file:
                self.status.config(text="Conversion cancelled")
                return
                
            # Read and process data
            self.status.config(text="Processing data...")
            self.root.update()
            
            # Read CSV with proper handling
            df = pd.read_csv(input_file)
            
            # Clean data - remove duplicate headers if present
            df = df[~df.iloc[:, 0].astype(str).str.contains('date , time')]
            
            # Convert date/time
            if 'date' in df.columns and 'time' in df.columns:
                # Handle zero values (0,0) and proper timestamps (20250420,114905)
                mask = (df['date'] != 0) & (df['time'] != 0)
                
                # Create datetime objects for valid rows
                df['datetime'] = pd.to_datetime(
                    df['date'].astype(str).str.zfill(8) + df['time'].astype(str).str.zfill(6),
                    format='%Y%m%d%H%M%S',
                    errors='coerce'
                )
                
                # For rows with valid datetime
                df_valid = df[mask].copy()
                df_valid['Date'] = df_valid['datetime'].dt.date
                df_valid['Time'] = df_valid['datetime'].dt.time
                
                # For rows with 0,0 values
                df_zero = df[~mask].copy()
                df_zero['Date'] = 0
                df_zero['Time'] = 0
                
                # Combine back
                df = pd.concat([df_valid, df_zero])
                
                # Drop temporary columns
                df.drop(['date', 'time', 'datetime'], axis=1, inplace=True)
                
                # Reorder columns
                cols = ['Date', 'Time'] + [col for col in df.columns if col not in ['Date', 'Time']]
                df = df[cols]
            
            # Save to Excel
            self.status.config(text="Saving Excel file...")
            self.root.update()
            
            output_file = filedialog.asksaveasfilename(
                title="Save Excel Output",
                defaultextension=".xlsx",
                filetypes=[("Excel files", "*.xlsx")]
            )
            if not output_file:
                self.status.config(text="Conversion cancelled")
                return
            
            # Write with Excel formatting
            with pd.ExcelWriter(output_file, engine='openpyxl') as writer:
                df.to_excel(writer, index=False, sheet_name='SensorData')
                
                # Get the worksheet for formatting
                workbook = writer.book
                worksheet = writer.sheets['SensorData']
                
                # Format date column (A) as YYYY-MM-DD
                for cell in worksheet['A']:
                    if cell.row == 1:  # Skip header
                        continue
                    if cell.value != 0:  # Only format non-zero dates
                        cell.number_format = numbers.FORMAT_DATE_YYYYMMDD2
                
                # Format time column (B) as HH:MM:SS
                for cell in worksheet['B']:
                    if cell.row == 1:  # Skip header
                        continue
                    if cell.value != 0:  # Only format non-zero times
                        cell.number_format = numbers.FORMAT_DATE_TIME6
                
                # Set column widths
                worksheet.column_dimensions['A'].width = 12  # Date column
                worksheet.column_dimensions['B'].width = 10  # Time column
                
                # Format other columns
                for col in range(3, len(df.columns) + 1):
                    column_letter = chr(64 + col)  # Convert to Excel column letter
                    worksheet.column_dimensions[column_letter].width = 12
            
            success_msg = f"Success! Saved to {output_file}"
            self.status.config(text=success_msg, fg='green')
            
        except Exception as e:
            error_msg = f"Error: {str(e)}"
            self.status.config(text=error_msg, fg='red')
            messagebox.showerror(
                "Conversion Error",
                f"An error occurred:\n{str(e)}",
                parent=self.root
            )

if __name__ == "__main__":
    root = tk.Tk()
    
    # Center the window on screen
    window_width = 500
    window_height = 280
    screen_width = root.winfo_screenwidth()
    screen_height = root.winfo_screenheight()
    x = (screen_width // 2) - (window_width // 2)
    y = (screen_height // 2) - (window_height // 2)
    root.geometry(f'{window_width}x{window_height}+{x}+{y}')
    
    app = LiupeConverter(root)
    root.mainloop()
