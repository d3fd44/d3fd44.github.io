import { useEffect, useRef } from "react";

export default function RaylibCanvas() {
    const canvasRef = useRef(null);

    useEffect(() => {
        const script = document.createElement("script");

        script.src = `${import.meta.env.BASE_URL}wasm/bg.js`;
        script.async = true;

        script.onload = async () => {
            try {
                await window.createBgModule({ canvas: canvasRef.current });
            } catch (error) {
                console.error("Failed to initialize raylib:", error);
            }
        };

        script.onerror = () => {
            console.error("Failed to load raylib module");
        };

        document.body.appendChild(script);

        return () => {
            script.remove();
        };
    }, []);

    return (
        <canvas
            id="canvas"
            ref={canvasRef}
            className="raylib-canvas"
        />
    );
}
