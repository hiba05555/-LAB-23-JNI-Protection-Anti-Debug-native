package com.example.lab22_jnidemo;

import androidx.appcompat.app.AppCompatActivity;
import android.graphics.Color;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    public native String helloFromJNI();
    public native int factorial(int n);
    public native String reverseString(String s);
    public native int sumArray(int[] values);

    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView tvStatus = findViewById(R.id.tvStatus);
        TextView tvHello = findViewById(R.id.tvHello);
        TextView tvFact = findViewById(R.id.tvFact);
        TextView tvReverse = findViewById(R.id.tvReverse);
        TextView tvArray = findViewById(R.id.tvArray);

        HCSecurityManager security = new HCSecurityManager();
        boolean suspicious = security.isSuspicious();
        String statusMsg = security.getStatusMessage();

        if (suspicious) {
            tvStatus.setText(statusMsg);
            tvStatus.setTextColor(Color.RED);
            tvHello.setText("Fonction desactivee");
            tvFact.setText("Calcul bloque");
            tvReverse.setText("Inversion bloquee");
            tvArray.setText("Somme bloquee");
        } else {
            tvStatus.setText(statusMsg);
            tvStatus.setTextColor(Color.parseColor("#2E7D32"));

            tvHello.setText(helloFromJNI());

            int fact10 = factorial(10);
            tvFact.setText("Factoriel de 10 = " + fact10);

            String reversed = reverseString("JNI is powerful!");
            tvReverse.setText("Texte inverse : " + reversed);

            int[] numbers = {10, 20, 30, 40, 50};
            int sum = sumArray(numbers);
            tvArray.setText("Somme du tableau = " + sum);
        }
    }
}