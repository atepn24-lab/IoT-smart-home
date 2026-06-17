from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# ================= STATES =================

latest_data = {
    "temperature": 0,
    "humidity": 0,
    "light": False,
    "motion": False,
    "alarm": False,
    "securityArmed": True,
    "distance": 0,
    "gasDetected": False,
    "gasValue": 0
}

manual_mode = False
security_armed = True
website_light_state = False

gas_buzzer_enabled = True

# ================= WEBSITE =================

@app.route('/')
def home():

    return render_template("index.html")

# ================= RECEIVE ESP32 DATA =================

@app.route('/data', methods=['POST'])
def data():

    global latest_data

    latest_data = request.get_json(force=True)

    print("====================")
    print(latest_data)

    return jsonify({
        "success": True
    })

# ================= SEND DATA TO WEBSITE =================

@app.route('/status')
def status():

    return jsonify(latest_data)

# ================= LIGHT ON =================

@app.route('/light/on')
def light_on():

    global manual_mode
    global website_light_state

    manual_mode = True

    website_light_state = True

    print("LIGHT ON")

    return jsonify({
        "success": True
    })

# ================= LIGHT OFF =================

@app.route('/light/off')
def light_off():

    global manual_mode
    global website_light_state

    manual_mode = True

    website_light_state = False

    print("LIGHT OFF")

    return jsonify({
        "success": True
    })

# ================= AUTO MODE =================

@app.route('/light/auto')
def light_auto():

    global manual_mode

    manual_mode = False

    print("AUTO MODE")

    return jsonify({
        "success": True
    })

# ================= ARM SECURITY =================

@app.route('/security/arm')
def arm_security():

    global security_armed

    security_armed = True

    print("SECURITY ARMED")

    return jsonify({
        "success": True
    })

# ================= DISARM SECURITY =================

@app.route('/security/disarm')
def disarm_security():

    global security_armed

    security_armed = False

    print("SECURITY DISARMED")

    return jsonify({
        "success": True
    })

# ================= GAS BUZZER ON =================

@app.route('/gasbuzzer/on')
def gas_buzzer_on():

    global gas_buzzer_enabled

    gas_buzzer_enabled = True

    print("GAS BUZZER ENABLED")

    return jsonify({
        "success": True
    })

# ================= GAS BUZZER OFF =================

@app.route('/gasbuzzer/off')
def gas_buzzer_off():

    global gas_buzzer_enabled

    gas_buzzer_enabled = False

    print("GAS BUZZER DISABLED")

    return jsonify({
        "success": True
    })

# ================= SEND COMMANDS =================

@app.route('/light/state')
def light_state():

    return jsonify({

        "manualMode":
            manual_mode,

        "light":
            website_light_state,

        "securityArmed":
            security_armed,

        "gasBuzzer":
            gas_buzzer_enabled
    })

# ================= START SERVER =================

if __name__ == '__main__':

    app.run(
        host='0.0.0.0',
        port=2000,
        debug=True
    )