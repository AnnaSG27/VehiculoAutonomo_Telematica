
"""
Cliente Python para el Vehículo Autónomo
Interfaz gráfica con Tkinter
"""

import socket
import threading
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import time
import json

class VehicleClient:
    def __init__(self):
        self.socket = None
        self.connected = False
        self.authenticated = False
        self.token = None
        self.user_type = None
        self.running = False
        
        # Datos de telemetría
        self.telemetry_data = {
            'speed': 0.0,
            'battery': 100.0,
            'temperature': 0.0,
            'direction': 'UNKNOWN',
            'latitude': 0.0,
            'longitude': 0.0,
            'timestamp': 0
        }
        
        self.setup_gui()
        
    def setup_gui(self):
        self.root = tk.Tk()
        self.root.title("Cliente Vehículo Autónomo")
        self.root.geometry("800x600")
        self.root.resizable(True, True)
        
        # Estilo
        style = ttk.Style()
        style.configure('Title.TLabel', font=('Arial', 14, 'bold'))
        style.configure('Data.TLabel', font=('Arial', 12))
        style.configure('Status.TLabel', font=('Arial', 10))
        
        # Frame principal
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Configurar peso de filas y columnas
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        
        # --- SECCIÓN DE CONEXIÓN ---
        conn_frame = ttk.LabelFrame(main_frame, text="Conexión", padding="5")
        conn_frame.grid(row=0, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        ttk.Label(conn_frame, text="Servidor:").grid(row=0, column=0, sticky=tk.W)
        self.host_var = tk.StringVar(value="localhost")
        ttk.Entry(conn_frame, textvariable=self.host_var, width=15).grid(row=0, column=1, padx=(5, 10))
        
        ttk.Label(conn_frame, text="Puerto:").grid(row=0, column=2, sticky=tk.W)
        self.port_var = tk.StringVar(value="8080")
        ttk.Entry(conn_frame, textvariable=self.port_var, width=8).grid(row=0, column=3, padx=(5, 10))
        
        self.connect_btn = ttk.Button(conn_frame, text="Conectar", command=self.connect)
        self.connect_btn.grid(row=0, column=4, padx=(10, 0))
        
        # --- SECCIÓN DE AUTENTICACIÓN ---
        auth_frame = ttk.LabelFrame(main_frame, text="Autenticación", padding="5")
        auth_frame.grid(row=1, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        ttk.Label(auth_frame, text="Usuario:").grid(row=0, column=0, sticky=tk.W)
        self.username_var = tk.StringVar()
        ttk.Entry(auth_frame, textvariable=self.username_var, width=15).grid(row=0, column=1, padx=(5, 10))
        
        ttk.Label(auth_frame, text="Contraseña:").grid(row=0, column=2, sticky=tk.W)
        self.password_var = tk.StringVar()
        ttk.Entry(auth_frame, textvariable=self.password_var, show="*", width=15).grid(row=0, column=3, padx=(5, 10))
        
        self.auth_btn = ttk.Button(auth_frame, text="Autenticar", command=self.authenticate, state="disabled")
        self.auth_btn.grid(row=0, column=4, padx=(10, 0))
        
        # Estado de conexión
        self.status_var = tk.StringVar(value="Desconectado")
        ttk.Label(auth_frame, textvariable=self.status_var, style='Status.TLabel').grid(row=1, column=0, columnspan=5, pady=(5, 0))
        
        # --- SECCIÓN DE TELEMETRÍA ---
        telemetry_frame = ttk.LabelFrame(main_frame, text="Datos de Telemetría", padding="10")
        telemetry_frame.grid(row=2, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))
        
        # Velocidad
        ttk.Label(telemetry_frame, text="Velocidad:", style='Title.TLabel').grid(row=0, column=0, sticky=tk.W)
        self.speed_var = tk.StringVar(value="0.0 km/h")
        ttk.Label(telemetry_frame, textvariable=self.speed_var, style='Data.TLabel', foreground='blue').grid(row=0, column=1, sticky=tk.W, padx=(10, 0))
        
        # Batería
        ttk.Label(telemetry_frame, text="Batería:", style='Title.TLabel').grid(row=1, column=0, sticky=tk.W)
        self.battery_var = tk.StringVar(value="100.0%")
        ttk.Label(telemetry_frame, textvariable=self.battery_var, style='Data.TLabel', foreground='green').grid(row=1, column=1, sticky=tk.W, padx=(10, 0))
        
        # Temperatura
        ttk.Label(telemetry_frame, text="Temperatura:", style='Title.TLabel').grid(row=2, column=0, sticky=tk.W)
        self.temp_var = tk.StringVar(value="0.0°C")
        ttk.Label(telemetry_frame, textvariable=self.temp_var, style='Data.TLabel', foreground='red').grid(row=2, column=1, sticky=tk.W, padx=(10, 0))
        
        # Dirección
        ttk.Label(telemetry_frame, text="Dirección:", style='Title.TLabel').grid(row=3, column=0, sticky=tk.W)
        self.direction_var = tk.StringVar(value="UNKNOWN")
        ttk.Label(telemetry_frame, textvariable=self.direction_var, style='Data.TLabel', foreground='purple').grid(row=3, column=1, sticky=tk.W, padx=(10, 0))
        
        # Coordenadas
        ttk.Label(telemetry_frame, text="Ubicación:", style='Title.TLabel').grid(row=4, column=0, sticky=tk.W)
        self.location_var = tk.StringVar(value="0.000000, 0.000000")
        ttk.Label(telemetry_frame, textvariable=self.location_var, style='Data.TLabel', foreground='orange').grid(row=4, column=1, sticky=tk.W, padx=(10, 0))
        
        # Última actualización
        ttk.Label(telemetry_frame, text="Última actualización:", style='Title.TLabel').grid(row=5, column=0, sticky=tk.W)
        self.last_update_var = tk.StringVar(value="Nunca")
        ttk.Label(telemetry_frame, textvariable=self.last_update_var, style='Status.TLabel').grid(row=5, column=1, sticky=tk.W, padx=(10, 0))
        
        # --- SECCIÓN DE COMANDOS (SOLO ADMIN) ---
        self.commands_frame = ttk.LabelFrame(main_frame, text="Comandos de Control", padding="10")
        self.commands_frame.grid(row=2, column=1, sticky=(tk.W, tk.E, tk.N, tk.S), padx=(10, 0), pady=(0, 10))
        
        # Botones de comando
        self.cmd_speedup_btn = ttk.Button(self.commands_frame, text="Acelerar", command=lambda: self.send_command("SPEED_UP", "5"), state="disabled")
        self.cmd_speedup_btn.pack(fill=tk.X, pady=2)
        
        self.cmd_slowdown_btn = ttk.Button(self.commands_frame, text="Desacelerar", command=lambda: self.send_command("SLOW_DOWN", "5"), state="disabled")
        self.cmd_slowdown_btn.pack(fill=tk.X, pady=2)
        
        self.cmd_left_btn = ttk.Button(self.commands_frame, text="Girar Izquierda", command=lambda: self.send_command("TURN_LEFT", "45"), state="disabled")
        self.cmd_left_btn.pack(fill=tk.X, pady=2)
        
        self.cmd_right_btn = ttk.Button(self.commands_frame, text="Girar Derecha", command=lambda: self.send_command("TURN_RIGHT", "45"), state="disabled")
        self.cmd_right_btn.pack(fill=tk.X, pady=2)
        
        self.cmd_stop_btn = ttk.Button(self.commands_frame, text="Detener", command=lambda: self.send_command("STOP", ""), state="disabled")
        self.cmd_stop_btn.pack(fill=tk.X, pady=2)
        
        # Botón para listar usuarios
        self.list_users_btn = ttk.Button(self.commands_frame, text="Listar Usuarios", command=self.list_users, state="disabled")
        self.list_users_btn.pack(fill=tk.X, pady=(10, 2))
        
        # --- SECCIÓN DE LOG ---
        log_frame = ttk.LabelFrame(main_frame, text="Log de Mensajes", padding="5")
        log_frame.grid(row=3, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(0, 10))
        
        self.log_text = scrolledtext.ScrolledText(log_frame, height=10, state='disabled')
        self.log_text.pack(fill=tk.BOTH, expand=True)
        
        # Configurar pesos para redimensionamiento
        main_frame.rowconfigure(2, weight=1)
        main_frame.rowconfigure(3, weight=1)
        
        # Configurar cierre de ventana
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
    def connect(self):
        if self.connected:
            self.disconnect()
            return
            
        try:
            host = self.host_var.get()
            port = int(self.port_var.get())
            
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(30)  # Timeout largo para evitar desconexión prematura
            self.socket.connect((host, port))
            
            self.connected = True
            self.running = True
            self.connect_btn.config(text="Desconectar")
            self.auth_btn.config(state="normal")
            self.status_var.set("Conectado - No autenticado")
            
            # Iniciar hilo para recibir mensajes
            self.receive_thread = threading.Thread(target=self.receive_messages)
            self.receive_thread.daemon = True
            self.receive_thread.start()
            
            self.log_message(f"Conectado a {host}:{port}")
            
        except Exception as e:
            messagebox.showerror("Error de conexión", f"No se pudo conectar: {str(e)}")
            self.log_message(f"Error de conexión: {str(e)}")
            
    def disconnect(self):
        self.running = False
        self.connected = False
        self.authenticated = False
        if self.socket:
            self.log_message("[DEBUG] Cerrando socket y desconectando...")
            self.socket.close()
            self.socket = None
        self.connect_btn.config(text="Conectar")
        self.auth_btn.config(state="disabled")
        self.disable_command_buttons()
        self.status_var.set("Desconectado")
        self.log_message("Desconectado del servidor")
        
    def authenticate(self):
        if not self.connected:
            messagebox.showerror("Error", "Debe conectarse primero")
            return
            
        username = self.username_var.get()
        password = self.password_var.get()
        
        if not username or not password:
            messagebox.showerror("Error", "Ingrese usuario y contraseña")
            return
            
        try:
            auth_message = f"AUTH_REQUEST|{int(time.time())}|NULL|{username}:{password}|CHECKSUM\r\n"
            self.socket.send(auth_message.encode())
            self.log_message(f"Enviado: {auth_message.strip()}")
        except Exception as e:
            messagebox.showerror("Error", f"Error enviando autenticación: {str(e)}")
            
    def send_command(self, command, params):
        if not self.authenticated or not self.token:
            messagebox.showerror("Error", "Debe autenticarse como administrador")
            return
        try:
            cmd_message = f"COMMAND_REQUEST|{int(time.time())}|{self.token}|{command}:{params}|CHECKSUM\r\n"
            self.socket.send(cmd_message.encode())
            self.log_message(f"Comando enviado: {command} {params}")
        except Exception as e:
            messagebox.showerror("Error", f"Error enviando comando: {str(e)}")
            
    def list_users(self):
        if not self.authenticated or not self.token or self.user_type != "ADMIN":
            messagebox.showerror("Error", "Debe ser administrador")
            return
        try:
            list_message = f"LIST_USERS_REQUEST|{int(time.time())}|{self.token}|NULL|CHECKSUM\r\n"
            self.socket.send(list_message.encode())
            self.log_message("Solicitando lista de usuarios...")
        except Exception as e:
            messagebox.showerror("Error", f"Error solicitando usuarios: {str(e)}")
            
    def receive_messages(self):
        buffer = ""
        while self.running and self.connected:
            try:
                data = self.socket.recv(1024)
                self.log_message(f"[DEBUG] Bytes recibidos: {data}")
                if not data:
                    self.log_message("[DEBUG] No se recibieron datos, cerrando conexión.")
                    break
                try:
                    decoded = data.decode()
                except Exception as e:
                    self.log_message(f"[ERROR] Fallo al decodificar datos: {str(e)}")
                    break
                self.log_message(f"[DEBUG] Cadena recibida: {decoded}")
                buffer += decoded
                while '\r\n' in buffer:
                    msg, buffer = buffer.split('\r\n', 1)
                    self.log_message(f"[DEBUG] Procesando mensaje separado: {msg}")
                    self.process_message(msg)
            except socket.timeout:
                self.log_message("[DEBUG] Timeout de socket, esperando más datos...")
                continue
            except Exception as e:
                if self.running:
                    self.log_message(f"[ERROR] Error recibiendo: {str(e)}")
                break
        # Si llega aquí, la conexión se perdió
        self.root.after(0, self.disconnect)
        
    def process_message(self, message):
        self.log_message(f"[DEBUG] Procesando mensaje: {message}")
        parts = message.split('|')
        self.log_message(f"[DEBUG] Partes del mensaje: {parts}")
        if len(parts) < 4:
            self.log_message(f"[ERROR] Mensaje recibido con formato incorrecto: {message}")
            return
        msg_type = parts[0]
        timestamp = parts[1]
        token = parts[2]
        data = parts[3]
        self.log_message(f"[DEBUG] Tipo: {msg_type}, Timestamp: {timestamp}, Token: {token}, Data: {data}")
        if msg_type == "AUTH_RESPONSE":
            self.log_message(f"[DEBUG] Procesando AUTH_RESPONSE: {data}")
            if ":" in data:
                user_type, status = data.split(':', 1)
                self.log_message(f"[DEBUG] user_type: {user_type}, status: {status}")
                if status == "200":
                    self.authenticated = True
                    self.token = token
                    self.user_type = user_type
                    self.status_var.set(f"Autenticado como {user_type}")
                    self.log_message(f"[INFO] Autenticación exitosa como {user_type}")
                    if user_type == "ADMIN":
                        self.enable_command_buttons()
                    self.root.after(0, lambda: messagebox.showinfo("Éxito", f"Autenticado como {user_type}"))
                else:
                    self.log_message(f"[ERROR] Autenticación fallida")
                    self.root.after(0, lambda: messagebox.showerror("Error", "Autenticación fallida"))
        elif msg_type == "TELEMETRY":
            self.log_message(f"[DEBUG] Procesando TELEMETRY: {data}")
            self.process_telemetry(data)
        elif msg_type == "COMMAND_RESPONSE":
            self.log_message(f"[DEBUG] Procesando COMMAND_RESPONSE: {data}")
            if ":" in data:
                status, message = data.split(':', 1)
                self.log_message(f"[DEBUG] status: {status}, message: {message}")
                if status == "200":
                    self.log_message(f"Comando ejecutado: {message}")
                else:
                    self.log_message(f"Error en comando: {message}")
        elif msg_type == "LIST_USERS_RESPONSE":
            self.log_message(f"[DEBUG] Procesando LIST_USERS_RESPONSE: {data}")
            if ":" in data:
                count, users = data.split(':', 1)
                user_list = users.rstrip(',').split(',') if users else []
                self.show_users_window(user_list)
                
    def process_telemetry(self, data):
        try:
            # Formato: speed:battery:temperature:direction:latitude:longitude
            parts = data.split(':')
            if len(parts) >= 6:
                self.telemetry_data['speed'] = float(parts[0])
                self.telemetry_data['battery'] = float(parts[1])
                self.telemetry_data['temperature'] = float(parts[2])
                self.telemetry_data['direction'] = parts[3]
                self.telemetry_data['latitude'] = float(parts[4])
                self.telemetry_data['longitude'] = float(parts[5])
                self.telemetry_data['timestamp'] = time.time()
                
                # Actualizar GUI en el hilo principal
                self.root.after(0, self.update_telemetry_display)
        except Exception as e:
            self.log_message(f"Error procesando telemetría: {str(e)}")
            
    def update_telemetry_display(self):
        self.speed_var.set(f"{self.telemetry_data['speed']:.1f} km/h")
        
        battery = self.telemetry_data['battery']
        self.battery_var.set(f"{battery:.1f}%")
        
        # Cambiar color según nivel de batería
        if battery > 50:
            color = 'green'
        elif battery > 20:
            color = 'orange'
        else:
            color = 'red'
            
        self.temp_var.set(f"{self.telemetry_data['temperature']:.1f}°C")
        self.direction_var.set(self.telemetry_data['direction'])
        self.location_var.set(f"{self.telemetry_data['latitude']:.6f}, {self.telemetry_data['longitude']:.6f}")
        
        # Actualizar timestamp
        if self.telemetry_data['timestamp'] > 0:
            time_str = time.strftime('%H:%M:%S', time.localtime(self.telemetry_data['timestamp']))
            self.last_update_var.set(time_str)
            
    def enable_command_buttons(self):
        self.cmd_speedup_btn.config(state="normal")
        self.cmd_slowdown_btn.config(state="normal")
        self.cmd_left_btn.config(state="normal")
        self.cmd_right_btn.config(state="normal")
        self.cmd_stop_btn.config(state="normal")
        self.list_users_btn.config(state="normal")
        
    def disable_command_buttons(self):
        self.cmd_speedup_btn.config(state="disabled")
        self.cmd_slowdown_btn.config(state="disabled")
        self.cmd_left_btn.config(state="disabled")
        self.cmd_right_btn.config(state="disabled")
        self.cmd_stop_btn.config(state="disabled")
        self.list_users_btn.config(state="disabled")
        
    def show_users_window(self, users):
        users_window = tk.Toplevel(self.root)
        users_window.title("Usuarios Conectados")
        users_window.geometry("300x200")
        
        ttk.Label(users_window, text="Usuarios conectados:", font=('Arial', 12, 'bold')).pack(pady=10)
        
        users_frame = ttk.Frame(users_window)
        users_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        if users and users[0]:  # Si hay usuarios y el primer elemento no está vacío
            for i, user in enumerate(users, 1):
                if user.strip():  # Solo mostrar usuarios no vacíos
                    ttk.Label(users_frame, text=f"{i}. {user}").pack(anchor=tk.W)
        else:
            ttk.Label(users_frame, text="No hay usuarios conectados").pack()
            
    def log_message(self, message):
        timestamp = time.strftime('%H:%M:%S')
        log_entry = f"[{timestamp}] {message}\n"
        
        # Actualizar en el hilo principal
        def update_log():
            self.log_text.config(state='normal')
            self.log_text.insert(tk.END, log_entry)
            self.log_text.see(tk.END)
            self.log_text.config(state='disabled')
            
        self.root.after(0, update_log)
        
    def on_closing(self):
        if self.connected:
            self.disconnect()
        self.root.destroy()
        
    def run(self):
        # Credenciales de ejemplo en la interfaz
        self.username_var.set("admin")  # Cambiar por "observer" para probar modo observador
        self.password_var.set("admin123")  # Cambiar por "observer123" para observador
        
        self.root.mainloop()

if __name__ == "__main__":
    client = VehicleClient()
    client.run()
