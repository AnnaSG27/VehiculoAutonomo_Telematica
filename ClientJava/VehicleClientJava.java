/*
 * VehicleClientJava.java
 * Cliente Java para el Vehículo Autónomo
 * Interfaz gráfica desarrollada en Swing
 * Descripción: Permite la conexión, autenticación y monitoreo de telemetría de un vehículo autónomo.
 */
import javax.swing.*;
import java.awt.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.*;
import java.net.*;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Clase principal del cliente Java para el vehículo autónomo.
 * Extiende JFrame para la interfaz gráfica.
 */
public class VehicleClientJava extends JFrame {
    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;
    private boolean connected = false;
    private boolean authenticated = false;
    private String token = null;
    private String userType = null;

    private JTextField hostField, portField, usernameField;
    private JPasswordField passwordField;
    private JButton connectButton, authButton;
    private JLabel statusLabel;
    private JButton speedUpButton, slowDownButton, turnLeftButton, turnRightButton, stopButton, listUsersButton;
    private JLabel speedLabel, batteryLabel, temperatureLabel, directionLabel, locationLabel, lastUpdateLabel;
    private JTextArea logArea;
    private JScrollPane logScrollPane;

    /**
     * Constructor: Inicializa la interfaz gráfica y configura el evento de cierre.
     */
    public VehicleClientJava() {
        super("Cliente Vehículo Autónomo - Java");
        initializeGUI();
        setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                onClosing();
            }
        });
    }

    /**
     * Configura y construye la interfaz gráfica principal del cliente.
     * Incluye secciones para conexión, autenticación, telemetría, comandos y log de mensajes.
     */
    private void initializeGUI() {
        setLayout(new BorderLayout());
        JPanel mainPanel = new JPanel(new BorderLayout(10, 10));
        mainPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        add(mainPanel, BorderLayout.CENTER);

        JPanel topPanel = new JPanel(new GridLayout(3, 1, 5, 5));
        JPanel connectionPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        connectionPanel.setBorder(BorderFactory.createTitledBorder("Conexión"));
        connectionPanel.add(new JLabel("Servidor:"));
        hostField = new JTextField("localhost", 10);
        connectionPanel.add(hostField);
        connectionPanel.add(new JLabel("Puerto:"));
        portField = new JTextField("8080", 6);
        connectionPanel.add(portField);
        connectButton = new JButton("Conectar");
        connectButton.addActionListener(e -> toggleConnection());
        connectionPanel.add(connectButton);
        topPanel.add(connectionPanel);

        JPanel authPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        authPanel.setBorder(BorderFactory.createTitledBorder("Autenticación"));
        authPanel.add(new JLabel("Usuario:"));
        usernameField = new JTextField("admin", 10);
        authPanel.add(usernameField);
        authPanel.add(new JLabel("Contraseña:"));
        passwordField = new JPasswordField("admin123", 10);
        authPanel.add(passwordField);
        authButton = new JButton("Autenticar");
        authButton.setEnabled(false);
        authButton.addActionListener(e -> authenticate());
        authPanel.add(authButton);
        topPanel.add(authPanel);

        JPanel statusPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        statusLabel = new JLabel("Estado: Desconectado");
        statusLabel.setFont(statusLabel.getFont().deriveFont(Font.BOLD));
        statusPanel.add(statusLabel);
        topPanel.add(statusPanel);

        mainPanel.add(topPanel, BorderLayout.NORTH);

        JPanel centerPanel = new JPanel(new GridLayout(1, 2, 10, 0));
        JPanel telemetryPanel = new JPanel(new GridBagLayout());
        telemetryPanel.setBorder(BorderFactory.createTitledBorder("Datos de Telemetría"));
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.anchor = GridBagConstraints.WEST;
        gbc.insets = new Insets(5, 10, 5, 10);

        gbc.gridx = 0; gbc.gridy = 0;
        telemetryPanel.add(new JLabel("Velocidad:"), gbc);
        gbc.gridx = 1;
        speedLabel = new JLabel("0.0 km/h");
        speedLabel.setForeground(Color.BLUE);
        speedLabel.setFont(speedLabel.getFont().deriveFont(Font.BOLD, 14f));
        telemetryPanel.add(speedLabel, gbc);

        gbc.gridx = 0; gbc.gridy = 1;
        telemetryPanel.add(new JLabel("Batería:"), gbc);
        gbc.gridx = 1;
        batteryLabel = new JLabel("100.0%");
        batteryLabel.setForeground(Color.GREEN);
        batteryLabel.setFont(batteryLabel.getFont().deriveFont(Font.BOLD, 14f));
        telemetryPanel.add(batteryLabel, gbc);

        gbc.gridx = 0; gbc.gridy = 2;
        telemetryPanel.add(new JLabel("Temperatura:"), gbc);
        gbc.gridx = 1;
        temperatureLabel = new JLabel("0.0°C");
        temperatureLabel.setForeground(Color.RED);
        temperatureLabel.setFont(temperatureLabel.getFont().deriveFont(Font.BOLD, 14f));
        telemetryPanel.add(temperatureLabel, gbc);

        gbc.gridx = 0; gbc.gridy = 3;
        telemetryPanel.add(new JLabel("Dirección:"), gbc);
        gbc.gridx = 1;
        directionLabel = new JLabel("UNKNOWN");
        directionLabel.setForeground(new Color(128, 0, 128));
        directionLabel.setFont(directionLabel.getFont().deriveFont(Font.BOLD, 14f));
        telemetryPanel.add(directionLabel, gbc);

        gbc.gridx = 0; gbc.gridy = 4;
        telemetryPanel.add(new JLabel("Ubicación:"), gbc);
        gbc.gridx = 1;
        locationLabel = new JLabel("0.000000, 0.000000");
        locationLabel.setForeground(Color.ORANGE);
        locationLabel.setFont(locationLabel.getFont().deriveFont(Font.BOLD, 12f));
        telemetryPanel.add(locationLabel, gbc);

        gbc.gridx = 0; gbc.gridy = 5;
        telemetryPanel.add(new JLabel("Última actualización:"), gbc);
        gbc.gridx = 1;
        lastUpdateLabel = new JLabel("Nunca");
        lastUpdateLabel.setForeground(Color.GRAY);
        telemetryPanel.add(lastUpdateLabel, gbc);

        centerPanel.add(telemetryPanel);

        JPanel commandsPanel = new JPanel(new GridBagLayout());
        commandsPanel.setBorder(BorderFactory.createTitledBorder("Comandos de Control"));
        gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.HORIZONTAL;
        gbc.insets = new Insets(5, 10, 5, 10);

        gbc.gridx = 0; gbc.gridy = 0;
        speedUpButton = new JButton("Acelerar");
        speedUpButton.setEnabled(false);
        speedUpButton.addActionListener(e -> sendCommand("SPEED_UP", "5"));
        commandsPanel.add(speedUpButton, gbc);

        gbc.gridy = 1;
        slowDownButton = new JButton("Desacelerar");
        slowDownButton.setEnabled(false);
        slowDownButton.addActionListener(e -> sendCommand("SLOW_DOWN", "5"));
        commandsPanel.add(slowDownButton, gbc);

        gbc.gridy = 2;
        turnLeftButton = new JButton("Girar Izquierda");
        turnLeftButton.setEnabled(false);
        turnLeftButton.addActionListener(e -> sendCommand("TURN_LEFT", "45"));
        commandsPanel.add(turnLeftButton, gbc);

        gbc.gridy = 3;
        turnRightButton = new JButton("Girar Derecha");
        turnRightButton.setEnabled(false);
        turnRightButton.addActionListener(e -> sendCommand("TURN_RIGHT", "45"));
        commandsPanel.add(turnRightButton, gbc);

        gbc.gridy = 4;
        stopButton = new JButton("Detener");
        stopButton.setEnabled(false);
        stopButton.addActionListener(e -> sendCommand("STOP", ""));
        commandsPanel.add(stopButton, gbc);

        gbc.gridy = 5;
        gbc.insets = new Insets(15, 10, 5, 10);
        listUsersButton = new JButton("Listar Usuarios");
        listUsersButton.setEnabled(false);
        listUsersButton.addActionListener(e -> listUsers());
        commandsPanel.add(listUsersButton, gbc);

        centerPanel.add(commandsPanel);
        mainPanel.add(centerPanel, BorderLayout.CENTER);

        JPanel logPanel = new JPanel(new BorderLayout());
        logPanel.setBorder(BorderFactory.createTitledBorder("Log de Mensajes"));
        logPanel.setPreferredSize(new Dimension(0, 150));
        logArea = new JTextArea();
        logArea.setEditable(false);
        logArea.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 11));
        logArea.setBackground(Color.BLACK);
        logArea.setForeground(Color.GREEN);
        logScrollPane = new JScrollPane(logArea);
        logScrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        logPanel.add(logScrollPane, BorderLayout.CENTER);
        mainPanel.add(logPanel, BorderLayout.SOUTH);

        setSize(900, 700);
        setLocationRelativeTo(null);
        setResizable(true);
    }

    /**
     * Alterna entre conectar y desconectar el cliente del servidor.
     */
    private void toggleConnection() {
        if (connected) {
            disconnect();
        } else {
            connect();
        }
    }

    /**
     * Establece la conexión con el servidor usando los datos ingresados por el usuario.
     * Inicia el hilo de recepción de mensajes.
     */
    private void connect() {
        try {
            String host = hostField.getText().trim();
            int port = Integer.parseInt(portField.getText().trim());
            socket = new Socket(host, port);
            out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"), false);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
            connected = true;
            connectButton.setText("Desconectar");
            authButton.setEnabled(true);
            statusLabel.setText("Estado: Conectado - No autenticado");
            statusLabel.setForeground(Color.ORANGE);
            new Thread(this::receiveMessages).start();
            logMessage("Conectado a " + host + ":" + port);
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Error de conexión: " + e.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
            logMessage("Error de conexión: " + e.getMessage());
        }
    }

    /**
     * Finaliza la conexión con el servidor y actualiza el estado de la interfaz.
     */
    private void disconnect() {
        try {
            connected = false;
            authenticated = false;
            token = null;
            userType = null;
            if (socket != null && !socket.isClosed()) {
                socket.close();
            }
            connectButton.setText("Conectar");
            authButton.setEnabled(false);
            disableCommandButtons();
            statusLabel.setText("Estado: Desconectado");
            statusLabel.setForeground(Color.RED);
            logMessage("Desconectado del servidor");
        } catch (Exception e) {
            logMessage("Error al desconectar: " + e.getMessage());
        }
    }

    /**
     * Envía el mensaje de autenticación al servidor con las credenciales ingresadas.
     */
    private void authenticate() {
        if (!connected) {
            JOptionPane.showMessageDialog(this, "Debe conectarse primero", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }
        String username = usernameField.getText().trim();
        String password = new String(passwordField.getPassword()).trim();
        if (username.isEmpty() || password.isEmpty()) {
            JOptionPane.showMessageDialog(this, "Ingrese usuario y contraseña", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }
        try {
            long timestamp = System.currentTimeMillis() / 1000;
            String authBody = String.format("AUTH_REQUEST|%d|NULL|%s:%s|CHECKSUM", timestamp, username, password);
            String authMessageWithNewline = authBody + "\r\n";
            out.print(authMessageWithNewline);
            out.flush();
            logMessage("Enviado: " + authBody);
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Error enviando autenticación: " + e.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
        }
    }

    /**
     * Envía un comando de control al servidor si el usuario está autenticado como administrador.
     */
    private void sendCommand(String command, String params) {
        if (!authenticated || token == null) {
            JOptionPane.showMessageDialog(this, "Debe autenticarse como administrador", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }
        try {
            long timestamp = System.currentTimeMillis() / 1000;
            String cmdBody = String.format("COMMAND_REQUEST|%d|%s|%s:%s|CHECKSUM", timestamp, token, command, params);
            String cmdMessageWithNewline = cmdBody + "\r\n";
            out.print(cmdMessageWithNewline);
            out.flush();
            logMessage("Comando enviado: " + command + " " + params);
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Error enviando comando: " + e.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
        }
    }

    /**
     * Solicita al servidor la lista de usuarios conectados (solo para administradores).
     */
    private void listUsers() {
        if (!authenticated || token == null || !"ADMIN".equals(userType)) {
            JOptionPane.showMessageDialog(this, "Debe ser administrador", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }
        try {
            long timestamp = System.currentTimeMillis() / 1000;
            String listBody = String.format("LIST_USERS_REQUEST|%d|%s|NULL|CHECKSUM", timestamp, token);
            String listMessageWithNewline = listBody + "\r\n";
            out.print(listMessageWithNewline);
            out.flush();
            logMessage("Solicitando lista de usuarios...");
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Error solicitando usuarios: " + e.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
        }
    }

    /**
     * Hilo encargado de recibir y procesar los mensajes enviados por el servidor.
     * Procesa cada mensaje por separado usando el delimitador '\r\n'.
     */
    private void receiveMessages() {
        try {
            logMessage("[DEBUG] Hilo de recepción iniciado (modo bytes)");
            char[] buffer = new char[1024];
            int readBytes;
            while (connected && (readBytes = in.read(buffer)) != -1) {
                String raw = new String(buffer, 0, readBytes);
                logMessage("[DEBUG] Bytes recibidos: " + readBytes + ", contenido: " + raw.replace("\r", "<CR>").replace("\n", "<LF>"));
                // Opcional: procesar por líneas si se detecta \r\n
                // Si quieres procesar por líneas:
                String[] lines = raw.split("\r?\n");
                for (String line : lines) {
                    if (!line.trim().isEmpty()) {
                        logMessage("[DEBUG] Procesando línea: " + line);
                        processMessage(line);
                    }
                }
            }
            logMessage("[DEBUG] Hilo de recepción finalizado (socket cerrado o fin de stream)");
        } catch (Exception e) {
            logMessage("[DEBUG] Excepción en hilo de recepción: " + e);
            if (connected) {
                SwingUtilities.invokeLater(() -> {
                    logMessage("Error recibiendo: " + e.getMessage());
                    disconnect();
                });
            }
        }
    }

    /**
     * Procesa el mensaje recibido del servidor y actualiza la interfaz según el tipo de mensaje.
     * Tipos: AUTH_RESPONSE, TELEMETRY, COMMAND_RESPONSE, LIST_USERS_RESPONSE
     */
    private void processMessage(String message) {
        String[] parts = message.split("\\|");
        if (parts.length < 4) {
            return;
        }
        String msgType = parts[0];
        String timestamp = parts[1];
        String msgToken = parts[2];
        String data = parts[3];
        switch (msgType) {
            case "AUTH_RESPONSE":
                handleAuthResponse(msgToken, data);
                break;
            case "TELEMETRY":
                handleTelemetry(data);
                break;
            case "COMMAND_RESPONSE":
                handleCommandResponse(data);
                break;
            case "LIST_USERS_RESPONSE":
                handleListUsersResponse(data);
                break;
        }
    }

    /**
     * Procesa la respuesta de autenticación y actualiza el estado del cliente y la interfaz.
     */
    private void handleAuthResponse(String msgToken, String data) {
        if (data.contains(":")) {
            String[] authParts = data.split(":", 2);
            String userTypeResp = authParts[0];
            String status = authParts[1];
            logMessage("[DEBUG] handleAuthResponse: userTypeResp=" + userTypeResp + ", status=" + status + ", msgToken=" + msgToken);
            if ("200".equals(status)) {
                authenticated = true;
                token = msgToken;
                logMessage("[DEBUG] Autenticación exitosa. authenticated=" + authenticated + ", token=" + token);
                if ("ADMIN".equals(userTypeResp) || "0".equals(userTypeResp)) {
                    userType = "ADMIN";
                    logMessage("[DEBUG] userType=ADMIN, habilitando botones de comando");
                    enableCommandButtons();
                } else if ("OBSERVER".equals(userTypeResp) || "1".equals(userTypeResp)) {
                    userType = "OBSERVER";
                    logMessage("[DEBUG] userType=OBSERVER");
                } else {
                    userType = userTypeResp;
                    logMessage("[DEBUG] userType desconocido: " + userType);
                }
                statusLabel.setText("Estado: Autenticado como " + userType);
                statusLabel.setForeground(Color.GREEN);
                logMessage("[DEBUG] statusLabel actualizado: Autenticado como " + userType);
                JOptionPane.showMessageDialog(this, "Autenticado como " + userType, "Éxito", JOptionPane.INFORMATION_MESSAGE);
            } else {
                logMessage("[DEBUG] Autenticación fallida. status=" + status);
                JOptionPane.showMessageDialog(this, "Autenticación fallida", "Error", JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    /**
     * Procesa los datos de telemetría recibidos y actualiza las variables correspondientes.
     */
    private void handleTelemetry(String data) {
        try {
            String[] parts = data.split(":");
            if (parts.length >= 6) {
                float speed = Float.parseFloat(parts[0]);
                float battery = Float.parseFloat(parts[1]);
                float temperature = Float.parseFloat(parts[2]);
                String direction = parts[3];
                float latitude = Float.parseFloat(parts[4]);
                float longitude = Float.parseFloat(parts[5]);
                updateTelemetryDisplay(speed, battery, temperature, direction, latitude, longitude);
            }
        } catch (Exception e) {
            logMessage("Error procesando telemetría: " + e.getMessage());
        }
    }

    /**
     * Procesa la respuesta de comando y muestra el resultado en el log.
     */
    private void handleCommandResponse(String data) {
        if (data.contains(":")) {
            String[] parts = data.split(":", 2);
            String status = parts[0];
            String message = parts[1];
            if ("200".equals(status)) {
                logMessage("Comando ejecutado: " + message);
            } else {
                logMessage("Error en comando: " + message);
            }
        }
    }

    /**
     * Procesa la respuesta de lista de usuarios y muestra la ventana correspondiente.
     */
    private void handleListUsersResponse(String data) {
        if (data.contains(":")) {
            String[] parts = data.split(":", 2);
            String count = parts[0];
            String users = parts[1];
            String[] userList = users.isEmpty() ? new String[0] : users.split(",");
            showUsersDialog(userList);
        }
    }

    /**
     * Actualiza los valores mostrados en la interfaz gráfica con los datos de telemetría actuales.
     */
    private void updateTelemetryDisplay(float speed, float battery, float temperature, String direction, float latitude, float longitude) {
        speedLabel.setText(String.format("%.1f km/h", speed));
        batteryLabel.setText(String.format("%.1f%%", battery));
        if (battery > 50) {
            batteryLabel.setForeground(Color.GREEN);
        } else if (battery > 20) {
            batteryLabel.setForeground(Color.ORANGE);
        } else {
            batteryLabel.setForeground(Color.RED);
        }
        temperatureLabel.setText(String.format("%.1f°C", temperature));
        directionLabel.setText(direction);
        locationLabel.setText(String.format("%.6f, %.6f", latitude, longitude));
        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");
        lastUpdateLabel.setText(sdf.format(new Date()));
    }

    /**
     * Muestra una ventana con la lista de usuarios conectados al sistema.
     */
    private void showUsersDialog(String[] users) {
        StringBuilder userList = new StringBuilder();
        userList.append("Usuarios conectados:\n\n");
        if (users.length == 0 || (users.length == 1 && users[0].trim().isEmpty())) {
            userList.append("No hay usuarios conectados");
        } else {
            int count = 1;
            for (String user : users) {
                if (!user.trim().isEmpty()) {
                    userList.append(count++).append(". ").append(user.trim()).append("\n");
                }
            }
        }
        JOptionPane.showMessageDialog(this, userList.toString(), "Usuarios Conectados", JOptionPane.INFORMATION_MESSAGE);
    }

    /**
     * Habilita los botones de comandos de control para el usuario administrador.
     */
    private void enableCommandButtons() {
        speedUpButton.setEnabled(true);
        slowDownButton.setEnabled(true);
        turnLeftButton.setEnabled(true);
        turnRightButton.setEnabled(true);
        stopButton.setEnabled(true);
        listUsersButton.setEnabled(true);
    }

    /**
     * Deshabilita los botones de comandos de control para usuarios no administradores.
     */
    private void disableCommandButtons() {
        speedUpButton.setEnabled(false);
        slowDownButton.setEnabled(false);
        turnLeftButton.setEnabled(false);
        turnRightButton.setEnabled(false);
        stopButton.setEnabled(false);
        listUsersButton.setEnabled(false);
    }

    /**
     * Agrega un mensaje al log de la interfaz gráfica, con timestamp.
     */
    private void logMessage(String message) {
        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");
        String timestamp = sdf.format(new Date());
        String logEntry = String.format("[%s] %s%n", timestamp, message);
        SwingUtilities.invokeLater(() -> {
            logArea.append(logEntry);
            logArea.setCaretPosition(logArea.getDocument().getLength());
        });
    }

    /**
     * Maneja el evento de cierre de la ventana principal, asegurando la desconexión limpia.
     */
    private void onClosing() {
        if (connected) {
            disconnect();
        }
        System.exit(0);
    }

    /**
     * Punto de entrada principal del cliente Java.
     */
    public static void main(String[] args) {
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {}
        SwingUtilities.invokeLater(() -> {
            new VehicleClientJava().setVisible(true);
        });
    }
}